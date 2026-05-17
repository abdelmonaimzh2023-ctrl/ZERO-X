#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <dirent.h>

struct FolderEntry {
    std::string relativePath;
    std::string fullPath;
    bool isDir;
    long long size;
    std::string type;
};

std::vector<FolderEntry> scanFolder(const std::string& rootPath) {
    std::vector<FolderEntry> entries;
    
    std::function<void(const std::string&, const std::string&)> scan = [&](const std::string& base, const std::string& current) {
        DIR* dir = opendir(current.c_str());
        if (!dir) return;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name == "." || name == "..") continue;
            
            std::string fullPath = current + "/" + name;
            std::string relativePath = base.empty() ? name : base + "/" + name;
            
            struct stat st;
            stat(fullPath.c_str(), &st);
            
            if (S_ISDIR(st.st_mode)) {
                FolderEntry fe;
                fe.relativePath = relativePath;
                fe.fullPath = fullPath;
                fe.isDir = true;
                fe.size = 0;
                fe.type = "DIR";
                entries.push_back(fe);
                scan(relativePath, fullPath);
            } else {
                FolderEntry fe;
                fe.relativePath = relativePath;
                fe.fullPath = fullPath;
                fe.isDir = false;
                fe.size = st.st_size;
                fe.type = getFileType(name);
                entries.push_back(fe);
            }
        }
        closedir(dir);
    };
    
    scan("", rootPath);
    return entries;
}

// Folder compression - packs all files into one .zp archive
bool compressFolder(const std::string& folderPath, const std::string& outputPath,
                    std::function<void(int, double, long long, const std::string&)> progressCallback) {
    auto entries = scanFolder(folderPath);
    
    progressCallback(10, 0, 999, "SCANNING FOLDER...");
    
    long long totalSize = 0;
    for (auto& e : entries) {
        if (!e.isDir) totalSize += e.size;
    }
    
    progressCallback(20, 0, 999, "FOLDER: " + std::to_string(entries.size()) + " items, " + 
                     Progress::formatSize(totalSize));
    
    // Write folder archive header
    std::ofstream outFile(outputPath, std::ios::binary);
    outFile << "ZPF"; // ZP Folder magic
    outFile.write(reinterpret_cast<const char*>(&totalSize), sizeof(long long));
    
    int fileCount = 0;
    for (auto& e : entries) {
        if (!e.isDir) fileCount++;
    }
    outFile.write(reinterpret_cast<const char*>(&fileCount), sizeof(int));
    
    // Write index
    for (auto& e : entries) {
        int pathLen = e.relativePath.length();
        outFile.write(reinterpret_cast<const char*>(&pathLen), sizeof(int));
        outFile.write(e.relativePath.c_str(), pathLen);
        outFile.write(reinterpret_cast<const char*>(&e.size), sizeof(long long));
        
        int typeLen = e.type.length();
        outFile.write(reinterpret_cast<const char*>(&typeLen), sizeof(int));
        outFile.write(e.type.c_str(), typeLen);
    }
    
    // Compress each file
    int processed = 0;
    for (auto& e : entries) {
        if (e.isDir) continue;
        
        processed++;
        int pct = 20 + (processed * 75 / fileCount);
        progressCallback(pct, 0, 999, "COMPRESSING: " + e.relativePath);
        
        if (isCompressible(e.type)) {
            // Compress with 12 layers
            compressFile(e.fullPath, "/tmp/zp_temp", [](int, double, long long, const std::string&){});
            // Append to archive
            std::ifstream temp("/tmp/zp_temp", std::ios::binary);
            outFile << temp.rdbuf();
            temp.close();
            remove("/tmp/zp_temp");
        } else {
            // Store as-is (images, etc.)
            std::ifstream src(e.fullPath, std::ios::binary);
            outFile << src.rdbuf();
            src.close();
        }
    }
    
    outFile.close();
    progressCallback(100, 0, 0, "FOLDER COMPRESSION COMPLETE");
    return true;
}
