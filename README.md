# C++ Shared Memory Code Sample

> This project is Linux specific, and will not work in Windows. It _might_ work
> on OSX, but I haven't tried. ~ KN

This repository contains an extremely quick and dirty example of using shared
memory to pass data between two processes in Linux.

The code is not intended to follow absolute best practices, it is intended to
be highly readable in order to walk you through creating and managing a shared
memory segment in C++ (or C with a bit of adaptation).

## What You Need

* A C++-supporting compiler toolchain, I recommend `gcc` or `clang`.
* Thats it!

## Building the Project

To build run one of the two following commands:

### GCC

    $ g++ main.cpp

### Clang

    $ clang++ main.cpp

## Run The App

In order to see the shared memory in action, you'll need to run two copies of
the application:

    Terminal 1:
    ----------

    $ ./a.out
    Send message (type 'exit' to quit):

    Terminal 2:
    ----------

    $ ./a.out -c
    Hit return when ready to read...

Write some text into the the first terminal, then in the second terminal press
any key to show the same message in the second process:

    Terminal 1:
    ----------

    $ ./a.out
    Send message (type 'exit' to quit): test123
    Sending 'test123' (length: 7)
    Hit return when ready to read...

    Terminal 2:
    ----------

    $ ./a.out -c
    Hit return when ready to read...
    Received: test123

Afterwards, you can do the reverse, and write a message into terminal 2, and see
it displayed in terminal 1.

Note that we don't do any special synchronization or anything here. Interprocess
communication is a fairly deep topic. Here we give the user the power to
synchronize the message passing between terminal 1 and terminal 2, but in a
production system you'd probably use sockets (e.g. unix domain sockets) or
something similar to wait for messages.

## Things To Try

* What happens if you enter a string longer than 100 characters?
  * Why?
  * How would you protect from overflows?
* What happens if you launch more than two copies of the program?

## License

This code is released into the public domain. You can copy, edit, and
re-release this code without guilt, and you don't have to give any credit.
