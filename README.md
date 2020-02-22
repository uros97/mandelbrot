# Mandelbrot set
Mandelbrot project for HPC class

How to use:

mandelbrot.cpp is source code for OpenACC version of code.
mandelbrot_threads.cpp is source code for sequential and concurrent version of code.

To move around the picture use W,S,A,D keyboard keys, and to zoom in and out use Equals and Dash keys (two keys left of Backspace key)

In files, there are executables for OpenACC version of code (mandelbrot) and same version of code compiled to work on host instead of GPU (mandelbrot_host)

Compilation requirements:

PGI pgc++ compiler (I used version 19.10)
SFML C++ library for drawing images (I used version 2.5.1)

Compilation:

If you installed SFML library in default location (/usr/lib/) then use following command to compile:

pgc++ -acc -fast -o mandelbrot -ta=tesla:managed -Minfo=accel mandelbrot.cpp -lsfml-graphics -lsfml-window -lsfml-system

Otherwise, give full path to the libraries required for compilation.

Enjoy!
