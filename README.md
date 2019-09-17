This is derivative of [Goptical - GNU Optical design and simulation library](https://www.gnu.org/software/goptical/) by Alexandre Becoulet.

Forked from [Goptical fork by Dmitry Galchinsky](https://github.com/galchinsky/goptical).

## Changes from original

* Different directory structure
* CMake instead of autotools

## Compiling on Linux Mint 

Tested for Linux Mint 18.3, but should also work for other Ubuntu-type Linux distributions:

```
     $ sudo apt-get install libgsl-dev libgsl2 gsl-bin libgd3 libgd-tools libgd-dev libplplot-dev libplplot-c++11 freeglut3 freeglut3-dev libopencv-dev libdime-dev libxmu-dev libxmuu-dev
```

First, clone the github repo by:

```
     $ git clone https://github.com/galchinsky/goptical.git
     $ cd goptical
```

Then create a directory `build` and `cd` there:

```
     $ mkdir build
     $ cd build
```

For creating the Makefiles via `cmake`, use:

```
     $ cmake ../
```

After successfully creating the Makefiles, perform `make`:

```
     $ make
```

As a result, you should find lots of examples in the `examples` sub directory which can be executed and the plots can be inspected by using, e.g., `inkscape`.

## Compiling on Redhat 7.7

#### Install pre-requisites

```
sudo yum install gsl-devel.x86_6
sudo yum install gd-devel
sudo yum install plplot-devel
sudo yum install mesa-libGL
sudo yum install freeglut-devel
sudo yum install opencv
sudo yum install opencv-devel
```

I had to disable DIME as I could not imstall the dependency.

I installed CMake manually.
Rest of the instructions are the same.



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
