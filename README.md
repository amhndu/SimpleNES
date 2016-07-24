SimpleNES
=============


An NES emulator written in C++ for purely educational purposes.
Currently WIP.
Only games with atmost 2 program banks and 1 character banks and no "mapper" are supported,
and only one controller is setup at the moment, with no scrolling so unfortunately no Super Mario Bros. :(
Examples of games that run (but not limited to):

* Mario Bros.
* Donkey Kong Jr.
* Balloon Fight
* Tennis


Compiling
-----------

You need:
* SFML 2.0+ development headers and library
* C++11 compliant compiler
* CMake build system

Compiling is straight forward with cmake, just run cmake on the project directory with CMAKE_BUILD_TYPE=Release
and you'll get Makefile or equivalent for your platform, with which you can compile the emulator

For e.g., on Linux/OS X/BSDs:
```
$ cd SimpleNES
$ mkdir build/ && cd build/
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j4    #Replace 4 with however many cores you have to spare

Running
-----------------

Just pass the path a .nes image
like
```
$ ./SimpleNES ~/Games/Mario\ Bros.\ \(World\).nes
```

Controller
-----------------

These keybindings are hard-coded (for Player 1)

 Button        | Mapped to
 --------------|-------------
 Start         | Return/Enter
 Select        | Right Shift
 A             | Numpad 4
 B             | Numpad 5
 Up            | W
 Down          | S
 Left          | A
 Righy         | D