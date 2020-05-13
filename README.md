![build](https://github.com/dibyendumajumdar/goptical/workflows/build/badge.svg)

This is derivative of [Goptical - GNU Optical design and simulation library](https://www.gnu.org/software/goptical/) by Alexandre Becoulet. Forked via [Goptical fork by Dmitry Galchinsky](https://github.com/galchinsky/goptical).

## Changes from original (done by Dmitry)

* Different directory structure
* Namespaces are lower case
* CMake instead of autotools

## Additional changes (this project)

* DONE Windows/MSVC port - remove use of features unsupported by MSVC such as VLAs. 
* DONE Disable all output options other than SVG for portability reasons (other output options may be enabled later)
* Mostly DONE Embed required components from GNU Scientific Library in the project (support for multi variable fitting 
  and ODE to be added)
* DONE Remove all external dependencies
* DRAFT add a driver program that can generate output from a specificatin given in a text file.
* DRAFT add support aspherical surfaces as specified in patent literature.
* DONE replace custom smart pointers with standard C++ smart pointers

## Changes to programming API

* The original implementation allowed mixing heap allocated and stack allocated data structures.
  We no longer allow that. All data structures except the top most one must be heap allocated and
  managed via smart pointers.

## Issues

* Some components are disabled for now - see the `CMakeLists.txt` in the `src\core` folder.

## Building on Linux

Create a directory `build` and `cd` there:

```
     $ mkdir build
     $ cd build
     $ cmake ..
     $ make
```

## Building on Windows

Similar to above except that `cmake` creates Visual Studio solution, so you can build using Visual Studio 2019.

## Running Goptical on example lenses

See the data files in `data` folder. The lens prescriptions are taken from [Photons To Photos](https://www.photonstophotos.net/) 
site. 

To run `Goptical` on these data files, execute the `gopt` command line utility built under `cmd`. Just provide a data file as an argument. Output will be generated in the current folder.

## LICENSE


 Copyright (C) 2010-2011 Free Software Foundation, Inc
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
