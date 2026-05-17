# ZERO-SPACE

> Bit-Table Hyper Compression Engine

## Features

- Custom bit-table per file (maximum compression)
- 12-pattern detection layers (1 to 256 bytes)
- APK, DOCUMENT, TEXT, CSV, JSON, XML, CODE support
- Interactive file browser with search, goto, help
- Folder compression
- Automatic system-wide installation
- Self-updating via GitHub
- Operation logging
- Images, Audio, Video stored as-is
- Original files always preserved

## Quick Install (Termux)

```

pkg install git clang make -y
git clone https://github.com/abdelmonaimzh2023-ctrl/ZERO-X.git
cd ZERO-X
bash build/build.sh
cp zp $PREFIX/bin/zp && chmod +x $PREFIX/bin/zp
zp

```

## Usage

| Command | Action |
|:---|:---|
| zp | Interactive menu |
| zp -c input output.zp | Compress file/folder |
| zp -c --force input output.zp | Force compress any file |
| zp -d input.zp output | Decompress |
| zp -b | File browser |
| zp -i file.zp | Show .zp file info |
| zp --version | Show version |
| zp --update | Check for updates |
| zp --help | Show help |

## Compression Performance

| File Type | Expected Ratio |
|:---|:---|
| Text, CSV, JSON, XML | 85-95% |
| Source Code | 80-92% |
| APK | 15-30% |
| PDF, DOCX | 10-40% |
| Images, Audio, Video | 0% (stored as-is) |

## Version

Current: 2.0.1 (BIT-STORM)
