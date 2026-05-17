#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <sys/ioctl.h>
#include <unistd.h>
#include <chrono>
#include <thread>

namespace Progress {

int getTermWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return (w.ws_col > 0) ? w.ws_col : 80;
}

std::string repeat(const std::string& s, int count) {
    std::string out;
    for (int i = 0; i < count; i++) out += s;
    return out;
}

std::string center(const std::string& text, int width) {
    int pad = (width - text.length()) / 2;
    if (pad < 0) pad = 0;
    return std::string(pad, ' ') + text;
}

std::string formatSize(long long bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = bytes;
    while (size >= 1024 && unit < 4) { size /= 1024; unit++; }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unit];
    return oss.str();
}

std::string formatTime(long long seconds) {
    if (seconds < 60) return std::to_string(seconds) + "s";
    if (seconds < 3600) return std::to_string(seconds / 60) + "m " + std::to_string(seconds % 60) + "s";
    return std::to_string(seconds / 3600) + "h " + std::to_string((seconds % 3600) / 60) + "m";
}

void clear() {
    std::cout << "\033[2J\033[H" << std::flush;
}

void hideCursor() {
    std::cout << "\033[?25l" << std::flush;
}

void showCursor() {
    std::cout << "\033[?25h" << std::flush;
}

// ANSI Color codes
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string DIM     = "\033[2m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
    
    std::string gradient(int percent) {
        if (percent < 33) return RED;
        if (percent < 66) return YELLOW;
        return GREEN;
    }
}

void legendaryBar(int percent, double speedMBps, long long eta, 
                  long long ramUsed, long long totalRam, double temp,
                  const std::string& status, long long saved, double ratio,
                  const std::string& filePath, bool isFolder = false) {
    clear();
    int w = getTermWidth();
    int barWidth = std::max(30, w - 45);
    int filled = (percent * barWidth) / 100;
    
    std::string bar = Color::gradient(percent) + repeat("\xE2\x96\x8C", std::min(filled, barWidth));
    if (filled < barWidth) bar += Color::DIM + repeat("\xE2\x96\x91", barWidth - filled);
    
    std::string icon = isFolder ? "\xF0\x9F\x93\x82" : "\xF0\x9F\x97\x9C\xEF\xB8\x8F";
    
    std::cout << Color::CYAN << Color::BOLD;
    std::cout << "\xE2\x95\x94" << repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\x97\n";
    std::cout << "\xE2\x95\x91" << center(icon + " ZERO-SPACE HYPER COMPRESSION", w - 2) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\xA0" << repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\xA3\n";
    std::cout << Color::RESET;
    
    std::cout << Color::WHITE << "\xE2\x95\x91  \xE2\xA0\xBF Status: " << Color::GREEN << status
              << repeat(" ", std::max(0, w - 22 - (int)status.length())) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  \xE2\xA0\xBF Stage:  " << Color::YELLOW << percent << "%"
              << repeat(" ", std::max(0, w - 20)) << "\xE2\x95\x91\n" << Color::RESET;
    
    std::cout << Color::WHITE << "\xE2\x95\x91  " << Color::RESET << bar << Color::WHITE << "  \xE2\x95\x91\n" << Color::RESET;
    
    std::cout << Color::WHITE << "\xE2\x95\xA0" << repeat("\xE2\x94\x80", w - 2) << "\xE2\x95\xA3\n";
    std::cout << "\xE2\x95\x91  \xE2\xA0\xBF Speed:   " << Color::GREEN << std::fixed << std::setprecision(1) << speedMBps
              << " MB/s" << Color::WHITE << "     \xE2\xA0\xBF ETA:  " << Color::YELLOW << formatTime(eta)
              << repeat(" ", std::max(0, w - 47)) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  \xE2\xA0\xBF RAM:     " << Color::CYAN << formatSize(ramUsed) << "/" << formatSize(totalRam)
              << Color::WHITE << "   \xE2\xA0\xBF Temp: " << Color::gradient((int)temp) << std::fixed << std::setprecision(0) << temp << "\xC2\xB0" << "C"
              << Color::WHITE << repeat(" ", std::max(0, w - 44)) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  \xE2\xA0\xBF Saved:   " << Color::MAGENTA << formatSize(saved) << " (" << std::fixed << std::setprecision(1) << (ratio * 100) << "%)"
              << Color::WHITE << repeat(" ", std::max(0, w - 32)) << "\xE2\x95\x91\n";
    
    std::string shortPath = filePath;
    if ((int)shortPath.length() > w - 17) {
        shortPath = "..." + shortPath.substr(shortPath.length() - (w - 20));
    }
    std::cout << "\xE2\x95\x91  \xE2\xA0\xBF Input:   " << Color::BLUE << shortPath
              << Color::WHITE << repeat(" ", std::max(0, w - 17 - (int)shortPath.length())) << "\xE2\x95\x91\n";
    
    std::cout << "\xE2\x95\x9A" << repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\x9D" << Color::RESET << "\n";
    std::cout.flush();
}

} // namespace Progress
