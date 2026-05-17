#include <cstdint>
#include <vector>

uint32_t crc32_table[256];
bool crc32_initialized = false;

void crc32_init() {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320 : 0);
        }
        crc32_table[i] = crc;
    }
    crc32_initialized = true;
}

uint32_t crc32_calc(const std::vector<unsigned char>& data) {
    if (!crc32_initialized) crc32_init();
    uint32_t crc = 0xFFFFFFFF;
    for (auto byte : data) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}
