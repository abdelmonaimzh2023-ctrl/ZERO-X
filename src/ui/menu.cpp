#include <iostream>
#include <string>

void showMainMenu() {
    Progress::clear();
    int w = Progress::getTermWidth();
    
    std::cout << Progress::Color::CYAN << Progress::Color::BOLD;
    std::cout << "\xE2\x95\x94" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\x97\n";
    std::cout << "\xE2\x95\x91" << Progress::center("ZERO-SPACE v1.0.9", w - 2) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91" << Progress::center("PRIMORDIAL ENGINE", w - 2) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\xA0" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\xA3\n";
    std::cout << "\xE2\x95\x91" << Progress::repeat(" ", w - 2) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  " << Progress::Color::GREEN << "[1] > COMPRESS" << Progress::Color::WHITE 
              << Progress::repeat(" ", w - 19) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  " << Progress::Color::GREEN << "[2] < DECOMPRESS" << Progress::Color::WHITE 
              << Progress::repeat(" ", w - 21) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  " << Progress::Color::YELLOW << "[3] ? INFO" << Progress::Color::WHITE 
              << Progress::repeat(" ", w - 15) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  " << Progress::Color::CYAN << "[4] ^ UPDATE" << Progress::Color::WHITE 
              << Progress::repeat(" ", w - 17) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91  " << Progress::Color::RED << "[5] X EXIT" << Progress::Color::WHITE 
              << Progress::repeat(" ", w - 15) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x91" << Progress::repeat(" ", w - 2) << "\xE2\x95\x91\n";
    std::cout << "\xE2\x95\x9A" << Progress::repeat("\xE2\x95\x90", w - 2) << "\xE2\x95\x9D\n";
    std::cout << Progress::Color::RESET;
    std::cout << "\n  Choice: ";
}
