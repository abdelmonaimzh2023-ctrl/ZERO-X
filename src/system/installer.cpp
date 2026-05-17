#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

bool autoInstall() {
    char exePath[4096];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len != -1) exePath[len] = '\0';
    
    std::string currentPath(exePath);
    std::string targetPath = "/data/data/com.termux/files/usr/bin/zp";
    
    if (currentPath == targetPath) return true; // Already installed
    
    std::cout << "[*] First run: Installing ZERO-SPACE to system...\n";
    
    // Copy binary
    std::ifstream src(currentPath, std::ios::binary);
    std::ofstream dst(targetPath, std::ios::binary);
    dst << src.rdbuf();
    src.close(); dst.close();
    
    // Make executable
    chmod(targetPath.c_str(), 0755);
    
    std::cout << "[+] Installed. Type 'zp' from anywhere now.\n";
    return true;
}
