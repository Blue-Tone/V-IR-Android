UNAME = $(shell uname)
PWD = $(shell pwd)

############################# Linux
# libusb-1.0.20 を手動コンパイル, インストールしてから make
# ディストリビューションによってコマンドを変更

ifeq ($(UNAME),Linux)

V-IR: v-ir.c
#dynamiclink
#gcc v-ir.c -o V-IR -lusb-1.0 -lpthread  -DLINUX=1

#ubuntu
#gcc v-ir.c -o V-IR -DLINUX=1 -static /usr/local/lib/libusb-1.0.a /usr/lib/x86_64-linux-gnu/libpthread.a

#raspi
	gcc v-ir.c -o V-IR -DLINUX=1 -static /usr/local/lib/libusb-1.0.a /usr/lib/arm-linux-gnueabihf/libpthread.a

#centos
#gcc v-ir.c -o V-IR -lusb-1.0 -static -lpthread -lrt -DLINUX=1 -I/usr/local/include

clean:
	touch V-IR
	rm    V-IR
	touch \#dummyfile
	touch dummyfile~
	rm \#* *~
	perl -e 'use Cwd; $$_=getcwd(); /(\d+)/; print "#define VERSION \"$$1\"\n"' > version.h
endif

############################# FreeBSD
# ライブラリ不要
# 32bit 機で Make する
ifeq ($(UNAME),FreeBSD)

V-IR: v-ir.c
	echo === Make on 32bit machine. ===
	gcc v-ir.c -lusb -static -lpthread -o V-IR -DFreeBSD=1

clean:
	touch V-IR
	rm    V-IR
	touch \#dummyfile
	touch dummyfile~
	rm \#* *~
	perl -e 'use Cwd; $$_=getcwd(); /(\d+)/; print "#define VERSION \"$$1\"\n"' > version.h
endif

############################# MINGW32_NT-6.2
# libusb-1.0.20 を手動コンパイル, インストールしてから make

ifeq ($(UNAME),MINGW32_NT-6.2)

V-IR.exe: v-ir.c
	gcc -Wno-deprecated-declarations v-ir.c -static -o V-IR.exe /usr/local/lib/libusb-1.0.a  -I/usr/local/include/libusb-1.0/ -DMINGW32=1

clean:
	touch V-IR.exe V-IR
	rm    V-IR.exe V-IR
	perl -e 'use Cwd; $$_=getcwd(); /(\d+)/; print "#define VERSION \"$$1\"\n"' > version.h
endif


