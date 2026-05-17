#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

const std::string CURRENT_VERSION = "1.0.9";
const std::string VERSION_URL = "https://raw.githubusercontent.com/abdelmonaimzh2023-ctrl/ZERO-X/main/version.txt";
const std::string DOWNLOAD_URL = "https://github.com/abdelmonaimzh2023-ctrl/ZERO-X/releases/latest/download/zp-arm64";

std::string fetchVersion() {
    // Simplified: use curl or wget to fetch version
    std::string cmd = "curl -s " + VERSION_URL + " 2>/dev/null || wget -qO- " + VERSION_URL + " 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    char buf[32];
    std::string result;
    if (fgets(buf, sizeof(buf), pipe)) result = buf;
    pclose(pipe);
    // Trim newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
    return result;
}

void checkUpdate() {
    std::cout << "[*] Checking for updates...\n";
    std::string latest = fetchVersion();
    
    if (latest.empty()) {
        std::cout << "[!] Could not check for updates. No internet?\n";
        return;
    }
    
    if (latest == CURRENT_VERSION) {
        std::cout << "[+] Already up to date (v" << CURRENT_VERSION << ")\n";
        return;
    }
    
    std::cout << "[*] Update available: v" << latest << " (current: v" << CURRENT_VERSION << ")\n";
    std::cout << "[?] Install update? (y/n): ";
    char c;
    std::cin >> c;
    
    if (c == 'y' || c == 'Y') {
        std::cout << "[*] Downloading update...\n";
        std::string cmd = "curl -L -o /tmp/zp_new " + DOWNLOAD_URL + " 2>/dev/null || wget -O /tmp/zp_new " + DOWNLOAD_URL + " 2>/dev/null";
        system(cmd.c_str());
        
        // Backup current
        system("cp /data/data/com.termux/files/usr/bin/zp /data/data/com.termux/files/usr/bin/zp.bak 2>/dev/null");
        
        // Install new
        system("cp /tmp/zp_new /data/data/com.termux/files/usr/bin/zp");
        system("chmod +x /data/data/com.termux/files/usr/bin/zp");
        system("rm /tmp/zp_new");
        
        std::cout << "[+] Update complete. Restart ZERO-SPACE.\n";
        exit(0);
    }
}
