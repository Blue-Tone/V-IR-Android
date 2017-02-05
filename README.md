# V-IR-Android

VIR-USB 接続赤外線学習リモコン  
http://purose.net/fanout/index.php?VIR-USB%20%E6%8E%A5%E7%B6%9A%E8%B5%A4%E5%A4%96%E7%B7%9A%E5%AD%A6%E7%BF%92%E3%83%AA%E3%83%A2%E3%82%B3%E3%83%B3


[Android] Android Studio をインストールする３つの手順（Windows）  
https://akira-watson.com/android/adt-windows.html


## ビルド方法
cd \V-IR-Android\src\v-ir
ndk-build clean
ndk-build 

\libs\armeabiに「v-ir」ができる。
実行時にlibusb1.0.soをライブラリとして読み込んでいる。

## androidにバイナリ転送
adb devicesで接続確認
接続できていれば、pushFiles.batを実行
(接続する端末により、転送パス・モジュールは変更。)

## 実行
export LD_LIBRARY_PATH=/data/local/tmp
cd /data/local/tmp
sh ./v-ir





## 参考メモ  

【Android】端末上で実行できる単独バイナリのコンパイル【C/C++】  
http://cubeundcube.blogspot.jp/2013/06/androidcc.html


Compile and link against libusb for android  
http://stackoverflow.com/questions/15957509/compile-and-link-against-libusb-for-android


libusbのgithub  
https://github.com/libusb/libusb

Android NDK でネイティブ CUI プログラムを書く！
http://dsas.blog.klab.org/archives/51809744.html
