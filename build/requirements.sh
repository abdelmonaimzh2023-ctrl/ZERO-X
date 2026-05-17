#!/bin/bash
echo "[*] Installing ZERO-SPACE dependencies..."
pkg update -y && pkg upgrade -y
pkg install clang make git -y
echo "[+] Dependencies installed."
