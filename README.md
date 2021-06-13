![build](https://github.com/dibyendumajumdar/goptical/workflows/build/badge.svg)

# NOTICE This project is now superceded by [Beam42](https://github.com/BeamFour/Beam42)

This is derivative of [Goptical - GNU Optical design and simulation library](https://www.gnu.org/software/goptical/) by Alexandre Becoulet. Forked via [Goptical fork by Dmitry Galchinsky](https://github.com/galchinsky/goptical).

## Changes from original (done by Dmitry)

* Different directory structure
* Namespaces are lower case
* CMake instead of autotools

## Additional changes (this project)

* DONE Windows/MSVC port - remove use of features unsupported by MSVC such as VLAs. 
* DONE Disable all output options other than SVG for portability reasons (other output options may be enabled later)
* Mostly DONE Embed required components from GNU Scientific Library in the project (support for multi variable fitting 
  and ODE to be added - see issue #17)
* DONE Remove all external dependencies
* DRAFT add a driver program that generates output from a specification given in a text file.
* DRAFT add support for aspherical surfaces as specified in patent literature. The implementation is based on Feder's paper - see Literature section below.
* DONE replace custom smart pointers with standard C++ smart pointers
* DONE compile as C++14
* DONE builds on Windows 10, Mac OSX and Linux

## Changes to programming API

* The original implementation allowed mixing heap allocated and stack allocated data structures.
  We no longer allow that. All data structures must be heap allocated and
  managed via smart pointers.

## Issues

* Some components are disabled for now - see the `CMakeLists.txt` in the `src\core` folder.
* The way chief ray is identified in GNU Optical is incorrect, as a result the ray fan analyses are also incorrect. 

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

## Documentation

* [Converted Original Docs](https://github.com/dibyendumajumdar/goptical/blob/master/documentation/goptical-manual.rst)

## Literature

* Donald P. Feder, "Optical Calculations with Automatic Computing Machinery," J. Opt. Soc. Am. 41, 630-635 (1951). This short paper provides equations for ray tracing for rotationally symmetric surfaces, including aspherics. Equations are provided in a format suitable for computer programs. Additionally, this paper also covers calculation of image aberrations. Warren J. Smith: Modern Optical Engineering book has a description of the algorithms in this paper. Note: The new implementation of rotationally symmetric aspheric surfaces in this project is based on this paper. 

* G. H. Spencer and M. V. R. K. Murty, "General Ray-Tracing Procedure," J. Opt. Soc. Am. 52, 672-678 (1962). This paper presents generalized ray tracing equations that cover not only rotationally symmetric surfaces (including aspherics) but also diffraction gratings. The paper allows for surfaces to have their own local axes. 

* R. E. Hopkins and R. Hanau, "Fundamental Methods of Ray Tracing," in Military Standardization Handbook: Optical Design, MIL-HDBK 141, U.S. Defense Supply Agency, Washington, DC, 1962. This is the fifth chapter in the document. It covers ray tracing equations for rotationally symmetric surfaces including aspheric surfaces. The equations are presented in a form suited for implementation in computer programs. The final equations in this document are very similar to Feder's equations. This document goes into details of how these equations are derived. Daniel Malacara: Handbook of Optical Design has a description of the ray tracing equations found in this document. 

* Bram de Greve, "Reflections and Refractions in Ray Tracing," 2004. This paper appears to be the source for the refraction equations originally used by GNU Optical. My plan is to use Feder's equations instead. 

* Telescope Optics - GNU Optical source code has references to this, it is unclear whether this is a reference to the book of this name by Rutten and Venrooij. It appears that the equations for intersection of rays with surfaces may have been based upon this; however my plan is to use Feder's equations instead.

## Related Projects

* Java port of GNU Optical: https://github.com/dibyendumajumdar/jfotoptix - work in progress
* .Net port of GNU Optical: https://github.com/dibyendumajumdar/nfotoptix - work in progress
* For a product developed by Michael Hayford who worked many years at optical software company - see https://github.com/mjhoptics/ray-optics. 
* Another Python project is https://github.com/quartiq/rayopt
* An older C project 'ray' originally written by Don Wells at NRAO implements Feder's equations for ray tracing. https://github.com/dibyendumajumdar/ray
* An attempt to maintain a commercial product KDP is here: https://github.com/dinosauria123/Koko. Lots of sphagetti Fortra code unfortunately. 

Here are some other projects that I have not personally tried out

* A new Julia project by Microsoft is https://github.com/microsoft/OpticSim.jl
* A C++ project: https://github.com/edeforas/Astree
* A Java project: https://github.com/StellarSoftwareBerkeley/BeamFour
* Python project OpticsSpy: https://github.com/Sterncat/opticspy 
* Another Python project https://github.com/mess42/pyrate
* A Matlab/Octave project: https://github.com/heterophyllus/OpticalDesign-Toolbox

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
