/*

      This file is part of the Goptical library.

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

/* -*- indent-tabs-mode: nil -*- */

#include <iostream>
#include <fstream>

#include <goptical/core/math/vector.hpp>

#include <goptical/core/sys/system.hpp>
#include <goptical/core/sys/source_point.hpp>
#include <goptical/core/sys/mirror.hpp>
#include <goptical/core/sys/image.hpp>

#include <goptical/core/shape/disk.hpp>

#include <goptical/core/curve/rotational.hpp>

#include <goptical/core/trace/tracer.hpp>
#include <goptical/core/trace/sequence.hpp>
#include <goptical/core/trace/distribution.hpp>
#include <goptical/core/trace/params.hpp>

#include <goptical/core/analysis/spot.hpp>
#include <goptical/core/analysis/focus.hpp>

//#include <goptical/core/io/RendererPlplot>
#include <goptical/core/io/renderer_svg.hpp>
#include <goptical/core/io/rgb.hpp>

#include <memory>

using namespace goptical;

/* anchor mycurve1 */
class MyCatenarycurve : public curve::Rotational
{
	public:
		MyCatenarycurve (double a) : _a (a) {}

	private:
		double sagitta (double r) const
		{
			return _a * cosh (r / _a) - _a;
		}
		/* anchor mycurve2 */
		double derivative (double r) const
		{
			return sinh (r / _a);
		}
		/* anchor mycurve1 */

		double _a;
};
/* anchor end */

int
main ()
{
	//**********************************************************************
	// Optical system definition
	/* anchor system */
	auto sys = std::make_shared<sys::System> ();
	// light source
	auto source = std::make_shared<sys::SourcePoint> (sys::SourceAtInfinity,
	              math::vector3_001);
	sys->add (source);
	// mirror
	auto shape = std::make_shared<shape::Disk> (200);
	auto curve = std::make_shared<MyCatenarycurve> (-3000);
	auto primary
	    = std::make_shared<sys::Mirror> (math::Vector3 (0, 0, 1500), curve, shape);
	sys->add (primary);
	// image plane
	auto image = std::make_shared<sys::Image> (math::vector3_0, 15);
	sys->add (image);
	/* anchor end */
	// set system entrance pupil
	sys->set_entrance_pupil (primary);
	auto seq = std::make_shared<trace::Sequence> ();
	seq->append (source);
	seq->append (primary);
	seq->append (image);
	sys->get_tracer_params ().set_sequential_mode (seq);
	sys->get_tracer_params ().set_default_distribution (
	    trace::Distribution (trace::HexaPolarDist, 8));
	{
		/* anchor focus */
		auto focus = std::make_shared<analysis::Focus> (sys);
		image->set_plane (focus->get_best_focus ());
		/* anchor end */
	}
	std::cout << seq << std::endl;
	{
		/* anchor spot */
		io::RendererSvg renderer ("spot.svg", 200 * 3, 200 * 2, io::rgb_black);
		renderer.set_margin_ratio (.35, .25, .1, .1);
		renderer.set_page_layout (3, 2);
		for (int i = 0; i < 6; i++)
		{
			analysis::Spot spot (sys);
			renderer.set_page (i);
			spot.draw_diagram (renderer);
			source->rotate (0, .1, 0);
		}
		/* anchor end */
		return 0;
	}
}
