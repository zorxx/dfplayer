# dfplayer
This library implements suport for the dfplayer mini serial MP3 module.

[![Build Status](https://travis-ci.org/zorxx/dfplayer.svg?branch=master)](https://travis-ci.org/zorxx/dfplayer)

This library is operating-system independent; it requires only a C99-compliant
toolchain to compile, so it can be used for any microcontroller or fully-adorned
operating system application. The serial read/write operations used for
communication with the dfplayer device aren't implemented in this library,
but are accomplished via asynchronous callback functions which are implemented
for the desired operating system target. Example applications for various
operating systems can be found in the examples directory.

All operations implemented in this library are executed asynchronously;
response and event handling are performed exclusively via callback functions.
This is particularly well-suited for microcontroller application with no
multi-tasking support.

For more information about this library please visit:
https://github.com/zorxx/dfplayer

### Reference

http://www.picaxe.com/docs/spe033.pdf 
