#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sys/stat.h>

namespace Logger {
    std::string logPath;
    
    void init() {
        std::string home = getenv("HOME") ? getenv("HOME") : "/data/data/com.termux/files/home";
        std::string logDir = home + "/.zero-space";
        mkdir(logDir.c_str(), 0700);
        logPath = logDir + "/zp.log";
    }
    
    std::string timestamp() {
        time_t now = time(0);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return std::string(buf);
    }
    
    void write(const std::string& msg) {
        std::ofstream log(logPath, std::ios::app);
        if (log.is_open()) {
            log << "[" << timestamp() << "] " << msg << std::endl;
        }
    }
}
