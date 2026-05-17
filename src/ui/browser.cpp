#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

struct FileEntry {
    std::string name;
    std::string fullPath;
    std::string type;   // DIR, TEXT, IMAGE, ARCHIVE, AUDIO, VIDEO, BINARY
    std::string icon;
    long long size;
    bool isDir;
};

std::vector<FileEntry> listDirectory(const std::string& path, bool showHidden = false) {
    std::vector<FileEntry> entries;
    DIR* dir = opendir(path.c_str());
    if (!dir) return entries;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == ".") continue;
        if (!showHidden && name[0] == '.') continue;
        
        FileEntry fe;
        fe.name = name;
        fe.fullPath = path + "/" + name;
        
        struct stat st;
        stat(fe.fullPath.c_str(), &st);
        
        if (S_ISDIR(st.st_mode)) {
            fe.isDir = true;
            fe.type = "DIR";
            fe.icon = "\xF0\x9F\x93\x81 "; // 📁
            fe.size = 0;
        } else {
            fe.isDir = false;
            fe.type = getFileType(name);
            fe.icon = getFileIcon(fe.type);
            fe.size = st.st_size;
        }
        entries.push_back(fe);
    }
    closedir(dir);
    
    // Sort: directories first, then alphabetically
    std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
        if (a.isDir != b.isDir) return a.isDir > b.isDir;
        return a.name < b.name;
    });
    
    return entries;
}

std::string fileBrowser(const std::string& startPath, const std::string& title, 
                        bool selectFile = true, bool selectFolder = false) {
    std::string currentPath = startPath;
    int page = 0;
    const int perPage = 15;
    
    while (true) {
        auto entries = listDirectory(currentPath);
        int totalPages = (entries.size() + perPage - 1) / perPage;
        if (totalPages == 0) totalPages = 1;
        if (page >= totalPages) page = 0;
        
        Progress::clear();
        int w = Progress::getTermWidth();
        
        // Header
        std::cout << Progress::Color::CYAN << Progress::Color::BOLD;
        std::cout << "\xE2\x95\x94" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\x97\n";
        std::cout << "\xE2\x95\x91 " << Progress::center(title, w - 2) << " \xE2\x95\x91\n";
        std::cout << "\xE2\x95\xA0" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\xA3\n";
        std::cout << Progress::Color::WHITE;
        std::cout << "\xE2\x95\x91 " << Progress::Color::YELLOW << "Current: " << currentPath 
                  << Progress::repeat(" ", std::max(0, w - 15 - (int)currentPath.length())) << "\xE2\x95\x91\n";
        std::cout << "\xE2\x95\xA0" << Progress::repeat("\xE2\x94\x80", w - 2) << "\xE2\x95\xA3\n";
        std::cout << Progress::Color::RESET;
        
        // Parent directory
        std::cout << Progress::Color::YELLOW << "\xE2\x95\x91  [..] \xF0\x9F\x93\x82 Parent Directory" 
                  << Progress::repeat(" ", std::max(0, w - 28)) << "\xE2\x95\x91\n" << Progress::Color::RESET;
        
        // Files
        int start = page * perPage;
        int end = std::min(start + perPage, (int)entries.size());
        
        for (int i = start; i < end; i++) {
            int num = i - start + 1;
            std::string numStr = (num < 10 ? " " : "") + std::to_string(num);
            std::string line = "[" + numStr + "] " + entries[i].icon + " " + entries[i].name;
            
            if (!entries[i].isDir) {
                line += "  " + Progress::formatSize(entries[i].size);
            }
            
            std::string color = entries[i].isDir ? Progress::Color::CYAN : Progress::Color::WHITE;
            std::cout << color << "\xE2\x95\x91  " << line 
                      << Progress::repeat(" ", std::max(0, w - 6 - (int)line.length())) << "\xE2\x95\x91\n";
        }
        
        // Fill empty space
        for (int i = end - start; i < perPage; i++) {
            std::cout << "\xE2\x95\x91" << Progress::repeat(" ", w - 2) << "\xE2\x95\x91\n";
        }
        
        std::cout << Progress::Color::RESET;
        
        // Footer
        std::cout << Progress::Color::WHITE;
        std::cout << "\xE2\x95\xA0" << Progress::repeat("\xE2\x94\x80", w - 2) << "\xE2\x95\xA3\n";
        std::cout << "\xE2\x95\x91  [" << Progress::Color::GREEN << "N" << Progress::Color::WHITE << "]ext Page  "
                  << "[" << Progress::Color::GREEN << "P" << Progress::Color::WHITE << "]rev Page  "
                  << "[" << Progress::Color::GREEN << "S" << Progress::Color::WHITE << "]earch  "
                  << "[" << Progress::Color::RED << "Q" << Progress::Color::WHITE << "]uit  "
                  << "Page " << (page + 1) << "/" << totalPages
                  << Progress::repeat(" ", std::max(0, w - 52)) << "\xE2\x95\x91\n";
        std::cout << "\xE2\x95\x9A" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\x9D\n";
        std::cout << Progress::Color::RESET;
        std::cout << "\n  Select (number/command): ";
        
        std::string input;
        std::cin >> input;
        
        if (input == "Q" || input == "q") {
            return "";
        } else if (input == "N" || input == "n") {
            if (page < totalPages - 1) page++;
        } else if (input == "P" || input == "p") {
            if (page > 0) page--;
        } else if (input == ".." || input == "0") {
            size_t pos = currentPath.find_last_of('/');
            if (pos != std::string::npos && pos > 0) {
                currentPath = currentPath.substr(0, pos);
                page = 0;
            }
        } else {
            try {
                int idx = std::stoi(input) - 1 + start;
                if (idx >= 0 && idx < (int)entries.size()) {
                    if (entries[idx].isDir) {
                        currentPath = entries[idx].fullPath;
                        page = 0;
                    } else if (selectFile) {
                        return entries[idx].fullPath;
                    }
                }
            } catch (...) {}
        }
    }
}
