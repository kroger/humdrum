This repository contains the "classical" humdrum toolkit.
Craig Sapp's humextra tools are now available on his [own github repository](https://github.com/craigsapp/humextra).
There are no binaries, I'll have to compile the code.

## Downloading Humdrum

To download the humdrum distribution go to the following page on the github repository:

  http://github.com/kroger/humdrum/

and clique on the "download" button.

If you are familiar with git, you may clone the whole repository:

    git clone git://github.com/kroger/humdrum.git

## Linux

To install humdrum on Linux you need to have a compiler and the alsa
development libraries installed. The following command should work on
ubuntu and debian:

  sudo apt-get install gcc g++ libasound2 libasound2-dev gawk

To compile, go to the humdrum directory and type the following commands:

    make
    sudo make install

## Mac OS

On the Mac OS make sure you have Xcode or Apple's Command Line Tools for Xcode.

To compile, go to the humdrum directory and type the following commands:

    make
    sudo make install

## Configuring

Put the following lines in your ~/.bashrc

    export PATH="/usr/local/humdrum/bin:$PATH"
    export AWK_VER=gawk
    export HUMDRUM=/usr/local/humdrum/
    export TMPDIR=/tmp
    export CON=/dev/tty
