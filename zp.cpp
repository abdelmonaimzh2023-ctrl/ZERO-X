#include <iostream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

// Include all component headers
// (In a real build, these would be .h files in include/)

// Forward declarations
void showMainMenu();
std::string fileBrowser(const std::string&, const std::string&, bool, bool);
CompressResult compressFile(const std::string&, const std::string&,
                           std::function<void(int, double, long long, const std::string&)>);
bool decompressFile(const std::string&, const std::string&);
bool compressFolder(const std::string&, const std::string&,
                    std::function<void(int, double, long long, const std::string&)>);
void checkUpdate();
bool autoInstall();
bool checkSystem();
void legendaryBar(int, double, long long, long long, long long, double,
                 const std::string&, long long, double, const std::string&, bool);

const std::string VERSION = "1.0.9";
const std::string BUILD_DATE = "2026-05-17";
const std::string CODENAME = "PRIMORDIAL";

void showHelp() {
    int w = Progress::getTermWidth();
    std::cout << Progress::Color::CYAN << Progress::Color::BOLD;
    std::cout << "\xE2\x95\x94" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\x97\n";
    std::cout << "\xE2\x95\x91" << Progress::center("ZERO-SPACE v" + VERSION + " - USAGE", w - 2) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\xA0" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\xA3\n";
    std::cout << Progress::Color::RESET;
    std::cout << Progress::Color::WHITE;
    std::cout << "\xE2\x95\x91  zp                       Interactive mode              \xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  zp -c <in> <out>         Compress file                 \xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  zp -d <in> <out>         Decompress file               \xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  zp -b                    File browser only             \xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  zp -i <file>             Show .zp file info            \xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  zp --version             Show version                  \xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  zp --update              Check for updates             \xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  zp --help                Show this help                \xE2\x95\x91\n";
    std::cout << "\xE2\x95\x9A" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\x9D\n";
    std::cout << Progress::Color::RESET;
}

void showVersion() {
    std::cout << "ZERO-SPACE v" << VERSION << "\n";
    std::cout << "Build: " << BUILD_DATE << "\n";
    std::cout << "Codename: " << CODENAME << "\n";
    std::cout << "Author: KHALIFA-AZL-7\n";
    std::cout << "Arch: " << 
        #ifdef __aarch64__
            "ARM64"
        #elif __arm__
            "ARM32"
        #else
            "Unknown"
        #endif
    << "\n";
    std::cout << "Engine: 12-layer hyper compression\n";
}

int main(int argc, char* argv[]) {
    // Auto-install on first run
    autoInstall();
    
    // Initialize logger
    Logger::init();
    Logger::write("ZERO-SPACE started");
    
    // Interactive mode (no arguments)
    if (argc == 1) {
        while (true) {
            showMainMenu();
            std::string choice;
            std::cin >> choice;
            
            if (choice == "1") {
                // Compress
                Logger::write("COMPRESS mode selected");
                std::string input = fileBrowser("/storage/emulated/0/Download", 
                                               "SELECT FILE TO COMPRESS", true, false);
                if (input.empty()) continue;
                
                std::string output = input + ".zp";
                
                Logger::write("COMPRESS START | " + input + " -> " + output);
                
                auto result = compressFile(input, output, 
                    [&](int pct, double speed, long long eta, const std::string& status) {
                        legendaryBar(pct, speed, eta, 0, 8000, 40, status, 0, 0, input, false);
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    });
                
                if (result.success) {
                    legendaryBar(100, result.speedMBps, 0, result.compressedSize, 8000, 42,
                               "COMPRESSION COMPLETE", result.originalSize - result.compressedSize,
                               result.ratio, input, false);
                    std::cout << Progress::Color::GREEN << "\n  [+] Success! ";
                    std::cout << Progress::formatSize(result.originalSize) << " -> ";
                    std::cout << Progress::formatSize(result.compressedSize) << " (";
                    std::cout << std::fixed << std::setprecision(1) << (result.ratio * 100) << "%)\n";
                    std::cout << "  [+] Time: " << (result.durationMs / 1000.0) << "s\n";
                    std::cout << Progress::Color::WHITE << "  [?] Press Enter to continue...";
                    std::cin.ignore(); std::cin.get();
                }
            }
            else if (choice == "2") {
                // Decompress
                Logger::write("DECOMPRESS mode selected");
                std::string input = fileBrowser("/storage/emulated/0/Download", 
                                               "SELECT .zp FILE TO DECOMPRESS", true, false);
                if (input.empty()) continue;
                
                std::string output = input + ".extracted";
                
                Logger::write("DECOMPRESS START | " + input + " -> " + output);
                
                // Show progress bar
                legendaryBar(50, 0, 5, 1024, 8000, 38, "DECOMPRESSING...", 0, 0, input, false);
                
                if (decompressFile(input, output)) {
                    legendaryBar(100, 0, 0, 1024, 8000, 40, "DECOMPRESSION COMPLETE", 0, 0, output, false);
                    Logger::write("DECOMPRESS END | SUCCESS");
                    std::cout << Progress::Color::GREEN << "\n  [+] Decompressed to: " << output << "\n";
                    std::cout << Progress::Color::WHITE << "  [?] Press Enter to continue...";
                    std::cin.ignore(); std::cin.get();
                }
            }
            else if (choice == "3") {
                showVersion();
                std::cout << "\n  [?] Press Enter to continue...";
                std::cin.ignore(); std::cin.get();
            }
            else if (choice == "4") {
                checkUpdate();
                std::cout << "\n  [?] Press Enter to continue...";
                std::cin.ignore(); std::cin.get();
            }
            else if (choice == "5" || choice == "x" || choice == "X") {
                Logger::write("ZERO-SPACE exited");
                break;
            }
        }
        return 0;
    }
    
    // CLI mode
    std::string cmd = argv[1];
    
    if (cmd == "--version") {
        showVersion();
    }
    else if (cmd == "--help") {
        showHelp();
    }
    else if (cmd == "--update") {
        checkUpdate();
    }
    else if (cmd == "-c" && argc >= 4) {
        std::string input = argv[2];
        std::string output = argv[3];
        
        struct stat st;
        if (stat(input.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            // Folder compression
            compressFolder(input, output, [](int, double, long long, const std::string&){});
            std::cout << "[+] Folder compressed: " << output << "\n";
        } else {
            // File compression
            auto result = compressFile(input, output, [](int, double, long long, const std::string&){});
            if (result.success) {
                std::cout << "[+] " << Progress::formatSize(result.originalSize) << " -> "
                          << Progress::formatSize(result.compressedSize) << " ("
                          << std::fixed << std::setprecision(1) << (result.ratio * 100) << "%)\n";
            }
        }
    }
    else if (cmd == "-d" && argc >= 4) {
        decompressFile(argv[2], argv[3]);
        std::cout << "[+] Decompressed: " << argv[3] << "\n";
    }
    else if (cmd == "-b") {
        std::string selected = fileBrowser("/storage/emulated/0/Download", "FILE BROWSER", false, false);
        if (!selected.empty()) std::cout << "Selected: " << selected << "\n";
    }
    else {
        showHelp();
    }
    
    Logger::write("ZERO-SPACE finished");
    return 0;
}
