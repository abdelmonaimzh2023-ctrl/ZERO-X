#include <string>
#include <algorithm>

std::string getFileType(const std::string& path) {
    std::string ext;
    size_t dot = path.find_last_of('.');
    if (dot != std::string::npos) {
        ext = path.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }
    
    // Images (not compressible)
    if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif" ||
        ext == "bmp" || ext == "webp" || ext == "heic" || ext == "ico" ||
        ext == "tiff" || ext == "svg") return "IMAGE";
    
    // Already compressed
    if (ext == "zip" || ext == "rar" || ext == "7z" || ext == "gz" ||
        ext == "xz" || ext == "bz2" || ext == "zst") return "ARCHIVE";
    if (ext == "mp3" || ext == "aac" || ext == "ogg" || ext == "flac" ||
        ext == "opus") return "AUDIO";
    if (ext == "mp4" || ext == "mkv" || ext == "avi" || ext == "webm") return "VIDEO";
    
    // Text-based (highly compressible)
    if (ext == "txt" || ext == "md" || ext == "log" || ext == "csv" ||
        ext == "json" || ext == "xml" || ext == "html" || ext == "css" ||
        ext == "js" || ext == "py" || ext == "c" || ext == "cpp" ||
        ext == "h" || ext == "java" || ext == "sh" || ext == "yml" ||
        ext == "yaml" || ext == "toml" || ext == "ini" || ext == "cfg" ||
        ext == "conf" || ext == "sql" || ext == "php" || ext == "rb") return "TEXT";
    
    return "BINARY";
}

std::string getFileIcon(const std::string& type) {
    if (type == "IMAGE") return "\xF0\x9F\x96\xBC\xEF\xB8\x8F "; // 🖼️
    if (type == "ARCHIVE") return "\xF0\x9F\x97\x9C\xEF\xB8\x8F "; // 🗜️
    if (type == "AUDIO") return "\xF0\x9F\x8E\xB5 "; // 🎵
    if (type == "VIDEO") return "\xF0\x9F\x8E\xAC "; // 🎬
    if (type == "TEXT") return "\xF0\x9F\x93\x84 "; // 📄
    return "\xF0\x9F\x93\x81 "; // 📁
}

bool isCompressible(const std::string& type) {
    return (type == "TEXT" || type == "BINARY");
}
