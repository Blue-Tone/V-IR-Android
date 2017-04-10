
rem set dst=sdcard
set dst=/data/local/tmp

set src=libs\armeabi
rem set src=libs\armeabi-v7a
rem mips
rem set src=libs\x86

adb push %src%\libusb1.0.so %dst%/
adb push %src%\v-ir %dst%/

pause