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

In addition, Pegasus3D dynamically loads shared libraries that implement additional Acorn types,
written using the C API. These libraries may have dependencies on additional code libraries,
such as [Asio][asio] (for Http and Udp), [libjpeg][jpeg] (for Jpeg), [giflib][gif] (for Gif), and [zlib][zip]
(for ZipFile).

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

Copyright (C) 2016  Jonathan Goodwin

 This program is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

[doc]: http://web3d.jondgoodwin.com/pegasusdoc
[web3d]: http://web3d.jondgoodwin.com
[doxygen]: http://doxygen.org
[acornvm]: http://web3d.jondgoodwin.com/arch/avmindex.html
[sdl2]: http://www.libsdl.org
[opengl]: http://www.opengl.org
[asio]: http://think-async.com/
[jpeg]: http://libjpeg.sourceforge.net/
[gif]: http://giflib.sourceforge.net/gif_lib.html
[zip]: http://www.zlib.net
