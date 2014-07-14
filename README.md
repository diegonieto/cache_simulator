Cache Simulator Lib
===================

This is a tiny library which plays as a cache simulator. The aim of this library is to compare the different behaviours of the cache memory.  Once you have executed the benchmark test you can check what is the difference between perform contiguous access to memory or not.

Usage
-----

    cd build
    make
    ./cache_simulator


Development
-----------
    
Follow header descriptor to develop your own code. You can set up to three parameters:
* Cache size
* Cache line
* Associtivity

Then, perform the access to the desired addresses and check the number of accesses and misses.
