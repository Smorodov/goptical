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


#include <Goptical/Material/Vacuum>

namespace _Goptical
{

	namespace Material
	{

		Vacuum::Vacuum()
		{
		}

		bool Vacuum::is_opaque() const
		{
			return false;
		}

		bool Vacuum::is_reflecting() const
		{
			return false;
		}

		double Vacuum::get_internal_transmittance(double wavelen, double thickness) const
		{
			return 1.0;
		}

		double Vacuum::get_extinction_coef(double wavelen) const
		{
			return 0.0;
		}

		double Vacuum::get_refractive_index(double wavelen) const
		{
			return 1.0;
		}

		Vacuum vacuum;
	}

}

