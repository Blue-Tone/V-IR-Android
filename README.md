# V-IR-Android

VIR-USB �ڑ��ԊO���w�K�����R��  
http://purose.net/fanout/index.php?VIR-USB%20%E6%8E%A5%E7%B6%9A%E8%B5%A4%E5%A4%96%E7%B7%9A%E5%AD%A6%E7%BF%92%E3%83%AA%E3%83%A2%E3%82%B3%E3%83%B3


[Android] Android Studio ���C���X�g�[������R�̎菇�iWindows�j  
https://akira-watson.com/android/adt-windows.html


## �r���h���@
cd \V-IR-Android\src\v-ir
ndk-build clean
ndk-build 

\libs\armeabi�Ɂuv-ir�v���ł���B
���s����libusb1.0.so�����C�u�����Ƃ��ēǂݍ���ł���B

## android�Ƀo�C�i���]��
adb devices�Őڑ��m�F
�ڑ��ł��Ă���΁ApushFiles.bat�����s
(�ڑ�����[���ɂ��A�]���p�X�E���W���[���͕ύX�B)

## ���s
export LD_LIBRARY_PATH=/data/local/tmp
cd /data/local/tmp
sh ./v-ir





## �Q�l����  

�yAndroid�z�[����Ŏ��s�ł���P�ƃo�C�i���̃R���p�C���yC/C++�z  
http://cubeundcube.blogspot.jp/2013/06/androidcc.html


Compile and link against libusb for android  
http://stackoverflow.com/questions/15957509/compile-and-link-against-libusb-for-android


libusb��github  
https://github.com/libusb/libusb

Android NDK �Ńl�C�e�B�u CUI �v���O�����������I
http://dsas.blog.klab.org/archives/51809744.html
