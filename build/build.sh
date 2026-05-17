#!/bin/bash
ARCH=$(uname -m)
echo "[*] Building ZERO-SPACE for $ARCH..."
clang++ -O3 -march=native -flto -pthread -std=c++17 zp.cpp -o zp
if [ $? -eq 0 ]; then
    echo "[+] Build successful: ./zp"
else
    echo "[-] Build failed"
    exit 1
fi
