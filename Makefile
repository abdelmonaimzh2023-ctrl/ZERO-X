.PHONY: all install clean update

all:
bash build/build.sh

install:
cp zp $(PREFIX)/bin/zp
chmod +x $(PREFIX)/bin/zp
@echo "[+] ZERO-SPACE installed. Type 'zp' to run."

clean:
rm -f zp *.o

update:
./zp --update
