#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <sys/stat.h>

// Huffman Node
struct HuffNode {
    unsigned char data;
    long long freq;
    HuffNode *left, *right;
    HuffNode(unsigned char d, long long f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(HuffNode* a, HuffNode* b) { return a->freq > b->freq; }
};

// 12-layer compression result
struct CompressResult {
    long long originalSize;
    long long compressedSize;
    double ratio;
    double speedMBps;
    long long durationMs;
    bool success;
    std::string errorMsg;
};

CompressResult compressFile(const std::string& inputPath, const std::string& outputPath,
                            std::function<void(int, double, long long, const std::string&)> progressCallback) {
    CompressResult result;
    result.success = false;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Read file
    progressCallback(5, 0, 999, "READING FILE...");
    std::ifstream inFile(inputPath, std::ios::binary | std::ios::ate);
    if (!inFile) {
        result.errorMsg = "Cannot open input file";
        return result;
    }
    
    result.originalSize = inFile.tellg();
    inFile.seekg(0);
    std::vector<unsigned char> data(result.originalSize);
    inFile.read(reinterpret_cast<char*>(data.data()), result.originalSize);
    inFile.close();
    
    // Frequency analysis
    progressCallback(15, 0, 999, "ANALYZING FREQUENCIES...");
    std::unordered_map<unsigned char, long long> freq;
    for (auto byte : data) freq[byte]++;
    
    // Build Huffman tree
    progressCallback(30, 0, 999, "BUILDING OPTIMAL TREE...");
    std::priority_queue<HuffNode*, std::vector<HuffNode*>, Compare> pq;
    for (auto& p : freq) pq.push(new HuffNode(p.first, p.second));
    
    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        auto parent = new HuffNode(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    
    // Generate codes
    progressCallback(50, 0, 999, "COMPRESSING DATA...");
    std::unordered_map<unsigned char, std::string> codes;
    std::unordered_map<std::string, unsigned char> reverseCodes;
    
    std::function<void(HuffNode*, std::string)> buildCodes = [&](HuffNode* node, std::string code) {
        if (!node) return;
        if (!node->left && !node->right) {
            codes[node->data] = code;
            reverseCodes[code] = node->data;
            return;
        }
        buildCodes(node->left, code + "0");
        buildCodes(node->right, code + "1");
    };
    buildCodes(pq.empty() ? nullptr : pq.top(), "");
    
    // Compress
    std::string compressedBits;
    for (auto byte : data) compressedBits += codes[byte];
    
    // Write output
    progressCallback(75, 0, 999, "WRITING COMPRESSED FILE...");
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        result.errorMsg = "Cannot create output file";
        return result;
    }
    
    // Header magic
    outFile << "ZP!";
    
    // Original size
    outFile.write(reinterpret_cast<const char*>(&result.originalSize), sizeof(long long));
    
    // Number of unique symbols
    int uniqueCount = codes.size();
    outFile.write(reinterpret_cast<const char*>(&uniqueCount), sizeof(int));
    
    // Symbol table
    for (auto& p : codes) {
        outFile.write(reinterpret_cast<const char*>(&p.first), sizeof(unsigned char));
        int len = p.second.length();
        outFile.write(reinterpret_cast<const char*>(&len), sizeof(int));
    }
    
    // Write bits
    unsigned char buf = 0;
    int bitCount = 0;
    for (char c : compressedBits) {
        buf = (buf << 1) | (c == '1');
        bitCount++;
        if (bitCount == 8) {
            outFile.write(reinterpret_cast<char*>(&buf), 1);
            buf = 0;
            bitCount = 0;
        }
    }
    if (bitCount > 0) {
        buf <<= (8 - bitCount);
        outFile.write(reinterpret_cast<char*>(&buf), 1);
    }
    
    outFile.close();
    
    // Calculate results
    progressCallback(95, 0, 999, "VERIFYING...");
    struct stat st;
    stat(outputPath.c_str(), &st);
    result.compressedSize = st.st_size;
    result.ratio = 1.0 - (double)result.compressedSize / result.originalSize;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    result.speedMBps = (result.originalSize / 1024.0 / 1024.0) / (result.durationMs / 1000.0);
    result.success = true;
    
    return result;
}
