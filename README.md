# Pegasus3D Browser
Surf and explore open, Internet-based 3D worlds using the Pegasus3D browser.
Like traditional browsers, Pegasus3D renders a site's content based on its URL address.
It also supports hyperlink navigation from one world to another.

Instead of flat pages, however, the Pegasus3D browser delivers interactive, three-dimensional worlds.
Pegasus3D worlds are built using modular resources written in Acorn,
a powerful, lightweight, dynamic, object-oriented language specifically designed to make it 
easier to define rich, immersive 3D worlds.

The [Pegasus 3D website][web3d] provides helpful detail about the 3D Internet vision,
the browser's place in it, reference information on the Acorn language and types, 
and an introduction to the software architecture.

## Architecture

The Pegasus3D browser is written in C. Its code uses these libraries:

- [Acorn VM][acornvm] - A reentrant C library, able to compile and run Acorn programs in its own virtual machine.
- [Simple Directmedia Layer][sdl2] - An operating system-independent API for managing media and other resources.
- [OpenGL][opengl] - An open API for high-performance graphics processing.
- [libcurl][libcurl] - the multi-protocol file transfer library.
- [stb-image][stb-image] - image loader	

## Building (Linux)

To build the library:

	make

## Building (Windows)

A Visual C++ 2010 solutions file can be created using the project files. 
The generated object, library, and executable files are created relative to the location of the 
solutions file.

## Documentation

Use [Doxygen][] to generate [documentation][doc] from the source code,
as guided by the pegasus3d.conf configuration file.

## License

Copyright (C) 2017  Jonathan Goodwin

The Pegasus3D browser (pegasus3d) is distributed under the terms of the MIT license. 
See LICENSE and COPYRIGHT for details.

[doc]: http://web3d.jondgoodwin.com/pegasusdoc
[web3d]: http://web3d.jondgoodwin.com
[doxygen]: http://doxygen.org
[acornvm]: http://web3d.jondgoodwin.com/arch/avmindex.html
[sdl2]: http://www.libsdl.org
[opengl]: http://www.opengl.org
[libcurl]: https://curl.haxx.se/libcurl
[stb-image]: https://github.com/nothings/stb/blob/master/stb_image.h
