#include <iostream>
#include <string>
#include <sys/sysinfo.h>
#include <unistd.h>

bool checkSystem() {
    struct sysinfo si;
    sysinfo(&si);
    long totalRAM = si.totalram / (1024 * 1024);
    
    if (totalRAM < 256) {
        std::cerr << "[!] WARNING: Low RAM (" << totalRAM << " MB). Minimum 256 MB recommended.\n";
    }
    
    // Check for required binaries
    if (system("which clang++ > /dev/null 2>&1") != 0) {
        std::cerr << "[!] clang++ not found. Run: pkg install clang\n";
        return false;
    }
    
    std::cout << "[+] System: OK | RAM: " << totalRAM << " MB | Cores: " 
              << sysconf(_SC_NPROCESSORS_ONLN) << "\n";
    return true;
}
