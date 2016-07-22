# Pegasus3D Browser

The Pegasus3D Browser supports the surfing and exploration of 3D worlds across the Internet,
much the way a traditional browser supports the surfing and exploration of pages.
The worlds exist as Internet resources written in Acorn, a dynamic object-oriented language.
The browser is built on top of the Acorn Virtual Machine.

## Documentation

[Doxygen][] can be used to generate [documentation][doc] from the source code.
The pegasus3d.conf Doxygen configuration file may be used to accomplish this.

This [website][web3d] offers additional information about the 3D Internet vision,
the browser's place in it, and architectural information.

## Building (Linux)

To build the library:

	make

## Building (Windows)

A Visual C++ 2010 solutions file can be created using the project files. 
The generated object, library, and executable files are created relative to the location of the 
solutions file.

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
