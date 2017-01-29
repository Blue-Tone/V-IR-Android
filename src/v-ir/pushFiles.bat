
set dst=sdcard
rem set dst=/data

rem set src=libs\armeabi
rem set src=libs\armeabi-v7a
rem mips
set src=libs\x86

adb push %src%\libusb1.0.so %dst%/v-ir
adb push %src%\v-ir %dst%/v-ir


rem adb push libs\armeabi\dpfp /sdcard/v-ir
rem adb push libs\armeabi\dpfp_threaded /sdcard/v-ir
rem adb push libs\armeabi\fxload /sdcard/v-ir
rem adb push libs\armeabi\hotplugtest /sdcard/v-ir
rem adb push libs\armeabi\listdevs /sdcard/v-ir
rem adb push libs\armeabi\sam3u_benchmark /sdcard/v-ir
rem adb push libs\armeabi\stress /sdcard/v-ir
rem adb push libs\armeabi\xusb /sdcard/v-ir



pause