* Make CPU time perfect
    - Read/write cycle accuracy
    - Instruction cycle length accuracy

* Make key bindings user-configurable (or load from a config file)

* Optimize the emulator and/or specifically PPU step function
    - Do something to prevent CPU cache thrashing
    - Caching tile/attribute/name-table data
    - Individual threads for CPU/PPU
    - Render per frame (with a queue for changes made to PPU state in between)
      or catch-up algorithm

* Overflow bit handling

* Handle palette mirroring properly (and completely)

* Fine x scrolling bug

* APU

* Add mappers

* GUI

* Add more comments