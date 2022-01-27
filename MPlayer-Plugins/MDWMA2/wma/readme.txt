DESCRIPTION:
------------

This is a  beta version of the WMA codec for the Roku PhotoBridge HD1000.
This codec will not decode WMA Professional, WMA Voice or WMA Lossless.

The wmadec.c file used is based on the ffmpeg-20041113 source.
The codec makes use of the MDCT routines from the Tremor Ogg Vorbis source code.
This codec also makes use of work done for WMA2WAV (Copyright (C) 2004, McMCC <mcmcc@mail.ru> )

AUTHOR
------

Peter McQuillan

Fixed point modifications by Derek Taubert (taubert@geeks.org) Feb 2005


INSTALLATION:
-------------

This package will only compile using SDK 2.0.16+ (i.e. needs at least version 2.0.16)

This package allows you to generate a test program or to generate a shared library file.

++++++
Option 1 - To make the test program

mv cascadecodec2wav.bak.used4developing cascadecodec2wav.cpp
cp Makefile.app Makefile
make

This will make an executable, the name of which depends on the name of the directory the files are in.

To run the program:

./mywma input.wma output.wav (in this case mywma was name of executable produced).

i.e. Pass two parameters the WMA file to decode and the name of the WAV file you want the result to be stored in

++++++
Option 2 - To make the shared library file

cp Makefile.lib Makefile
make

This will make a .so file based on the name of the directory.
Make sure to rename this file libCascadeAudioCodecWMA.so

To install the shared library

Step 1
======

Add the following line

wma:String:WMA

to the end of the file

/etc/rokucascade/settings/com.roku.deschutes.sonata.extensions


Step 2
======

Simple method is to copy the file

libCascadeAudioCodecWMA.so

to the directory

/etc/rokucascade/codecs

However, it is recommened that you store the WMA codec (as well as any other third party codecs) in a shared folder or on a compact flash card and put a symbolic link in the /etc/rokucascade/codecs directory linking to libCascadeAudioCodecWMA.so - this will keep space free in the internal storage area.

++++++

DISCLAIMER
----------

This product has been created independently of Roku, and therefore does not carry any express or implied Roku warranty and will not be supported by Roku Customer Support.
