/*

      This file is part of the <goptical/core library.

      The <goptical/core library is free software; you can redistribute it
      and/or modify it under the terms of the GNU General Public
      License as published by the Free Software Foundation; either
      version 3 of the License, or (at your option) any later version.

      The <goptical/core library is distributed in the hope that it will be
      useful, but WITHOUT ANY WARRANTY; without even the implied
      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
      See the GNU General Public License for more details.

      You should have received a copy of the GNU General Public
      License along with the <goptical/core library; if not, write to the
      Free Software Foundation, Inc., 59 Temple Place, Suite 330,
      Boston, MA 02111-1307 USA

      Copyright (C) 2010-2011 Free Software Foundation, Inc
      Author: Alexandre Becoulet

*/

/* -*- indent-tabs-mode: nil -*- */

#include <iostream>
#include <fstream>

#include <goptical/core/math/vector.hpp>

#include <goptical/core/material/abbe.hpp>

#include <goptical/core/sys/system.hpp>
#include <goptical/core/sys/lens.hpp>
#include <goptical/core/sys/source.hpp>
#include <goptical/core/sys/source_rays.hpp>
#include <goptical/core/sys/source_point.hpp>
#include <goptical/core/sys/image.hpp>
#include <goptical/core/sys/stop.hpp>

#include <goptical/core/curve/sphere.hpp>
#include <goptical/core/shape/disk.hpp>

#include <goptical/core/trace/tracer.hpp>
#include <goptical/core/trace/result.hpp>
#include <goptical/core/trace/distribution.hpp>
#include <goptical/core/trace/sequence.hpp>
#include <goptical/core/trace/params.hpp>

#include <goptical/core/light/spectral_line.hpp>

#include <goptical/core/analysis/rayfan.hpp>
#include <goptical/core/analysis/spot.hpp>
#include <goptical/core/analysis/focus.hpp>
#include <goptical/core/data/plot.hpp>

#include <goptical/core/io/renderer_svg.hpp>
#include <goptical/core/io/rgb.hpp>

using namespace goptical;

int main()
{
	//**********************************************************************
	// Optical system definition
	std::shared_ptr<sys::System>   sys = std::make_shared<sys::System>();
	/* anchor lens */
	auto lens = std::make_shared<sys::Lens>(math::Vector3(0, 0, 0));
	//               roc,            ap.radius, thickness,
	lens->add_surface(78.687,  38.00, 9.884,
	                  std::make_shared<material::AbbeVd>(1.8, 45.6));
	lens->add_surface(471.434,              38.0, 0.194);
	lens->add_surface(50.297, 32.0, 9.108,
	                  std::make_shared<material::AbbeVd>(1.773, 49.6));
	lens->add_surface(74.376,  31.0, 2.946);
	lens->add_surface(138.143, 30.0, 2.326,
	                  std::make_shared<material::AbbeVd>(1.673, 32.2));
	lens->add_surface(34.326,  25.5, 16.070);
	lens->add_stop(24.6, 13.0);
	lens->add_surface(-34.407,  24.4, 1.938,
	                  std::make_shared<material::AbbeVd>(1.74, 28.3));
	lens->add_surface(-2906.977,              28.5, 12.403,
	                  std::make_shared<material::AbbeVd>(1.773, 49.6));
	lens->add_surface(-59.047,  30.0, 0.388);
	lens->add_surface(-150.021, 33.4, 8.333,
	                  std::make_shared<material::AbbeVd>(1.788, 47.5));
	lens->add_surface(-57.890,  33.9, 0.194);
	lens->add_surface(284.630, 33.0, 5.039,
	                  std::make_shared<material::AbbeVd>(1.788, 47.5));
	lens->add_surface(-253.217,  33.0, 74.064);
	double image_pos = 9.884 + 0.194 + 9.108 + 2.946 + 2.326 + 16.070
	                   + 13.0 + 1.938 + 12.403 + 0.388 + 8.333 + 0.194 + 5.039 + 74.064;
	sys->add(lens);
	/* anchor end */
	auto image = std::make_shared<sys::Image>(math::Vector3(0, 0, image_pos), 42.42);
	sys->add(image);
	/* anchor sources */
	auto source_rays = std::make_shared<sys::SourceRays>(math::Vector3(0, 27.5, -1000));
	auto source_point = std::make_shared<sys::SourcePoint>(sys::SourceAtFiniteDistance,
	                    math::Vector3(0, 27.5, -1000));
	//  sys::SourcePoint source_point(sys::SourceAtInfinity,
	//				math::Vector3(0, 0, 1));
	// add sources to system
	sys->add(source_rays);
	sys->add(source_point);
	// configure sources
	source_rays->add_chief_rays(*sys);
	source_rays->add_marginal_rays(*sys, 14);
	source_point->clear_spectrum();
	source_point->add_spectral_line(light::SpectralLine::C);
	source_point->add_spectral_line(light::SpectralLine::e);
	source_point->add_spectral_line(light::SpectralLine::F);
	/* anchor end */
	/* anchor seq */
	auto seq = std::make_shared<trace::Sequence>(*sys);
	sys->get_tracer_params().set_sequential_mode(seq);
	std::cout << "system:" << std::endl << sys;
	std::cout << "sequence:" << std::endl << seq;
	/* anchor end */
	{
		/* anchor focus */
		analysis::Focus               focus(sys);
		image->set_plane(focus.get_best_focus());
		/* anchor end */
	}
	//**********************************************************************
	// Drawing rays and layout
	{
		/* anchor layout */
		io::RendererSvg renderer("layout.svg", 800, 400);
#if 1
		// draw 2d system layout
		sys->draw_2d_fit(renderer);
		sys->draw_2d(renderer);
#else
		// draw 2d layout of lens only
		lens.draw_2d_fit(renderer);
		lens.draw_2d(renderer);
#endif
		trace::Tracer tracer(sys.get());
		// trace and draw rays from rays source
		sys->enable_single<sys::Source>(*source_rays);
		tracer.get_trace_result().set_generated_save_state(*source_rays);
		tracer.trace();
		tracer.get_trace_result().draw_2d(renderer);
		/* anchor end */
	}
	{
		/* anchor spot */
		sys->enable_single<sys::Source>(*source_point);
		sys->get_tracer_params().set_default_distribution(
		    trace::Distribution(trace::HexaPolarDist, 12));
		analysis::Spot spot(sys);
		/* anchor end */
		{
			/* anchor spot */
			io::RendererSvg renderer("spot.svg", 300, 300, io::rgb_black);
			spot.draw_diagram(renderer);
			/* anchor end */
		}
		{
			/* anchor spot_plot */
			io::RendererSvg renderer("spot_intensity.svg", 640, 480);
			std::shared_ptr<data::Plot> plot = spot.get_encircled_intensity_plot(50);
			plot->draw(renderer);
			/* anchor end */
		}
	}
	{
		/* anchor opd_fan */
		sys->enable_single<sys::Source>(*source_point);
		analysis::RayFan fan(sys);
		/* anchor end */
		{
			/* anchor opd_fan */
			io::RendererSvg renderer("opd_fan.svg", 640, 480);
			std::shared_ptr<data::Plot> fan_plot = fan.get_plot(analysis::RayFan::EntranceHeight,
			                                       analysis::RayFan::OpticalPathDiff);
			fan_plot->draw(renderer);
			/* anchor end */
		}
		{
			/* anchor transverse_fan */
			io::RendererSvg renderer("transverse_fan.svg", 640, 480);
			std::shared_ptr<data::Plot> fan_plot = fan.get_plot(analysis::RayFan::EntranceHeight,
			                                       analysis::RayFan::TransverseDistance);
			fan_plot->draw(renderer);
			/* anchor end */
		}
		{
			/* anchor longitudinal_fan */
			io::RendererSvg renderer("longitudinal_fan.svg", 640, 480);
			std::shared_ptr<data::Plot> fan_plot = fan.get_plot(analysis::RayFan::EntranceHeight,
			                                       analysis::RayFan::LongitudinalDistance);
			fan_plot->draw(renderer);
			/* anchor end */
		}
	}
	return 0;
}

