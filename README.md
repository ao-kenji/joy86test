joy86test
=========

joy86test - A test program to tweak joystick port on NEC PC-9801-86

This is my experimental program to tweak joystick port on NEC PC-9801-86
sound board on OpenBSD/luna88k.  Now I test "Fighting Pad 6B" for
SEGA Mega Drive.

Preparation
-----------
To use this program, make sure that 'pcex0' is recognized in dmesg.
```
cbus0 at mainbus0
pcex0 at cbus0
```

Compile
-------
Build by 'make'.  The executable binary is 'joy86test'.

Run
---
```
% ./joy86test [-d]
```

Reference
---------
YM2608 OPNA アプリケーションマニュアル

UNDOCUMENTED 9801/9821 Vol.2 - メモリ・I/Oポート編 - サウンド関係I/O
    http://www.webtech.co.jp/company/doc/undocumented_mem/io_sound.txt

メガドライブパッドで東方旧作（自作アダプタ）
    http://j02.nobody.jp/jto98/n_desk_joystick/mdpa.htm

セガ メガドライブ　６ボタンパッドの読み取り方
    https://applause.elfmimi.jp/md6bpad.html
