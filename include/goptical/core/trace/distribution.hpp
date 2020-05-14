/*

      This file is part of the Goptical Core library.
  
      The Goptical library is free software; you can redistribute it
      and/or modify it under the terms of the GNU General Public
      License as published by the Free Software Foundation; either
      version 3 of the License, or (at your option) any later version.
  
      The Goptical library is distributed in the hope that it will be
      useful, but WITHOUT ANY WARRANTY; without even the implied
      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
      See the GNU General Public License for more details.
  
      You should have received a copy of the GNU General Public
      License along with the Goptical library; if not, write to the
      Free Software Foundation, Inc., 59 Temple Place, Suite 330,
      Boston, MA 02111-1307 USA
  
      Copyright (C) 2010-2011 Free Software Foundation, Inc
      Author: Alexandre Becoulet

*/

#ifndef GOPTICAL_TRACE_DISTRIBUTION_HH_
#define GOPTICAL_TRACE_DISTRIBUTION_HH_

#include "goptical/core/common.hpp"
#include "goptical/core/error.hpp"


namespace goptical
{

  namespace trace
  {

    /**
       @short Ray distribution pattern descriptor
       @header <goptical/core/trace/Distribution
       @module {Core}
       @main

       This class describes distribution pattern and ray density used
       for light ray distribution over surfaces during light
       propagation.

       Ray density is expressed as average number of rays along
       surface radius.

       @image dist_patterns.png {Different patterns rendered on a disk with default density}
     */

    class Distribution
    {
    public:
      /** Creates a distribution pattern with specified pattern,
          radial ray density and scaling.

          The scaling ratio parameter may be used to avoid
          distributing rays too close to the surface edge. */
      inline Distribution(Pattern pattern = DefaultDist,
                          unsigned int radial_density = 5,
                          double scaling = 0.999);

      /** Set distribution pattern */
      inline void set_pattern(Pattern p);

      /** Get distribution pattern */
      inline Pattern get_pattern() const;

      /** Set average number of ray to distribute on surface radius */
      inline void set_radial_density(unsigned int count);

      /** Get current radial density */
      inline unsigned int get_radial_density() const;

      /** Set scaling, Rays are distributed on surface but are
          kept away from surface edge if scaling is less than
          1.0. */
      inline void set_scaling(double scaling);

      /** get current scaling */
      inline double get_scaling() const;

      /** Ensure uniform 2d pattern suitable for spot diagram and
          image analysis is selected. Change to default pattern if
          not. */
      inline void set_uniform_pattern();

    private:
      Pattern           _pattern;
      unsigned int      _radial_density;
      double            _scaling;
    };

    Distribution::Distribution(Pattern pattern,
			       unsigned int radial_density,
			       double scaling)
      : _pattern(pattern),
	_radial_density(radial_density),
	_scaling(scaling)
    {
      if (radial_density < 1)
	throw Error("ray distribution radial density must be greater than 1");
    }

    void Distribution::set_pattern(Pattern p)
    {
      _pattern = p;
    }

    Pattern Distribution::get_pattern() const
    {
      return _pattern;
    }

    unsigned int Distribution::get_radial_density() const
    {
      return _radial_density;
    }

    void Distribution::set_radial_density(unsigned int density)
    {
      _radial_density = density;
    }

    double Distribution::get_scaling() const
    {
      return _scaling;
    }

    void Distribution::set_scaling(double margin)
    {
      _scaling = margin;
    }

    void Distribution::set_uniform_pattern()
    {
      switch (_pattern)
	{
      case SagittalDist:
      case MeridionalDist:
      case CrossDist:
	_pattern = DefaultDist;
      default:
	;
	}
    }

  }
}

#endif

