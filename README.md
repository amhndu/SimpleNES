SimpleNES
=============


An NES emulator written in C++ for nothing but fun.

Roughly 40-50% of games should work (ie. games that use either no mapper or mappers 1, 2, 3 and experimental support for 4, 7, 66 and 11).




Examples of games that have been tested to run (but NOT limited to):

(USA/Japan or World versions only i.e. NTSC compatible)

* Super Mario Bros.
* Contra
* Adventure Island
* Ninja Gaiden
* Wrecking Crew
* Megaman and Megaman 2
* Mario Bros.
* Donky Kong and Donkey Kong Jr.
* Battle City
* Paperboy
* Legend of Zelda
* Pacman
* Tennis
* Excitebike
* Nightmare Elm Street
* Cabal
* Battletoads
* Battletoads & Double Dragon
* Arch Rivals
* etc...


Screenshots
------------------------
![Screenshot 1](http://amhndu.github.io/screenshots/nes1.png)
![Screenshot 2](http://amhndu.github.io/screenshots/nes2.png)
![Screenshot 3](http://amhndu.github.io/screenshots/nes3.png)
![Screenshot 4](http://amhndu.github.io/screenshots/nes4.png)
![Screenshot 5](http://amhndu.github.io/screenshots/nes5.png)
![Screenshot 6](http://amhndu.github.io/screenshots/nes6.png)
![Screenshot 6](http://amhndu.github.io/screenshots/nes7.png)
![Screenshot 6](http://amhndu.github.io/screenshots/nes8.png)

Videos
------------
[YouTube Playlist](https://www.youtube.com/playlist?list=PLiULt7qySWt2VbHTkvIt9kYPMPcWt01qN)


Download
-----------
Executables:

[Windows 32-bit](https://www.dropbox.com/s/1gqhtbmvzo1ozsz/SimpleNES-win32.rar?dl=0)
[Linux 64-bit](https://www.dropbox.com/s/7eswcdektlkdz65/SimpleNES-linux64?dl=0)


Compiling
-----------

You need:
* SFML 2.0+ development headers and library
* C++11 compliant compiler
* CMake build system

Compiling is straight forward with cmake, just run cmake on the project directory with CMAKE_BUILD_TYPE=Release
and you'll get Makefile or equivalent for your platform, with which you can compile the emulator

For e.g., on Linux/OS X/FreeBSD:
```
$ git clone https://github.com/amhndu/SimpleNES
$ cd SimpleNES
$ mkdir build/ && cd build/
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j4    #Replace 4 with however many cores you have to spare
```

Running
-----------------

Just pass the path to a .nes image like

```
$ ./SimpleNES ~/Games/SuperMarioBros.nes
```
To set size of the window,
```
$ ./SimpleNES -w 600 ~/Games/Contra.nes
```
For supported command line options, try
```
$ ./SimpleNES -h
```

Controller
-----------------

Keybindings can be configured with keybindings.conf


Default keybindings:

**Player 1**

 Button        | Mapped to
 --------------|-------------
 Start         | Return/Enter
 Select        | Right Shift
 A             | J
 B             | K
 Up            | W
 Down          | S
 Left          | A
 Right         | D


**Player 2**

 Button        | Mapped to
 --------------|-------------
 Start         | Numpad9
 Select        | Numpad8
 A             | Numpad5
 B             | Numpad6
 Up            | Up
 Down          | Down
 Left          | Left
 Right         | Right


