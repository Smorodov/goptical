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

#include <fstream>
#include <iostream>

#include <goptical/core/math/vector.hpp>

#include <goptical/core/material/base.hpp>
#include <goptical/core/material/sellmeier.hpp>

#include <goptical/core/sys/image.hpp>
#include <goptical/core/sys/optical_surface.hpp>
#include <goptical/core/sys/source_point.hpp>
#include <goptical/core/sys/system.hpp>

#include <goptical/core/curve/sphere.hpp>
#include <goptical/core/shape/disk.hpp>

#include <goptical/core/trace/distribution.hpp>
#include <goptical/core/trace/params.hpp>
#include <goptical/core/trace/result.hpp>
#include <goptical/core/trace/sequence.hpp>
#include <goptical/core/trace/tracer.hpp>

#include <goptical/core/light/spectral_line.hpp>

#include <goptical/core/analysis/rayfan.hpp>
#include <goptical/core/data/plot.hpp>

#include <goptical/core/io/renderer_svg.hpp>
#include <goptical/core/io/rgb.hpp>

using namespace goptical;

int
main ()
{
  //**********************************************************************
  // Glass material models
  /* anchor material */
  auto bk7 = std::make_shared<material::Sellmeier> (1.03961212, 6.00069867e-3,
                                                    0.231792344, 2.00179144e-2,
                                                    1.01046945, 1.03560653e2);

  auto f3 = std::make_shared<material::Sellmeier> (
      8.23583145e-1, 6.41147253e-12, 7.11376975e-1, 3.07327658e-2,
      3.12425113e-2, 4.02094988);
  /* anchor end */

  //**********************************************************************
  // Optical system definition

  /* anchor lens_shape */
  auto lens_shape
      = std::make_shared<shape::Disk> (100); // lens diameter is 100mm

  // 1st lens, left surface
  auto curve1 = std::make_shared<curve::Sphere> (
      2009.753); // spherical curve with given radius of curvature
  auto curve2 = std::make_shared<curve::Sphere> (-976.245);
  /* anchor lens1 */
  auto s1 = std::make_shared<sys::OpticalSurface> (
      math::Vector3 (0, 0, 0), // position,
      curve1, lens_shape,      // curve & aperture shape
      material::none, bk7);    // materials

  // 1st lens, right surface
  auto s2 = std::make_shared<sys::OpticalSurface> (
      math::Vector3 (0, 0, 31.336), curve2, lens_shape, bk7, material::none);
  /* anchor lens2 */
  // 2nd lens, left surface
  auto s3 = std::make_shared<sys::OpticalSurface> (
      math::Vector3 (0, 0, 37.765), // position,
      -985.291, 100,                // roc & circular aperture radius,
      material::none, f3);          // materials

  // 2nd lens, right surface
  auto s4 = std::make_shared<sys::OpticalSurface> (
      math::Vector3 (0, 0, 37.765 + 25.109), -3636.839, 100, f3,
      material::none);
  /* anchor src */
  // light source
  auto source = std::make_shared<sys::SourcePoint> (sys::SourceAtInfinity,
                                                    math::Vector3 (0, 0, 1));
  /* anchor image */
  // image plane
  auto image
      = std::make_shared<sys::Image> (math::Vector3 (0, 0, 3014.5), // position
                                      60); // square size,
                                           /* anchor sys */
  auto sys = std::make_shared<sys::System> ();

  // add components
  sys->add (source);
  sys->add (s1);
  sys->add (s2);
  sys->add (s3);
  sys->add (s4);
  sys->add (image);
  /* anchor end */

  //**********************************************************************
  // Simple ray trace

#if 1
  std::cout << "Performing sequential raytrace" << std::endl;
  /* anchor seq */
  auto seq = std::make_shared<trace::Sequence> (*sys);

  sys->get_tracer_params ().set_sequential_mode (seq);
  /* anchor end */

  /* anchor print */
  std::cout << "system:" << std::endl << sys;
  std::cout << "sequence:" << std::endl << seq;
  /* anchor end */
#else
  std::cout << "Performing non-sequential raytrace" << std::endl;

  /* anchor nonseq */
  sys->set_entrance_pupil (s1);
  /* anchor end */

#endif
  {
    /* anchor trace */
    trace::Tracer tracer (sys.get ());
    tracer.trace ();
    /* anchor end */
  }

  //**********************************************************************
  // Drawing rays and layout

  {
    /* anchor layout */
    io::RendererSvg renderer ("layout.svg", 1024, 100);

    // draw 2d system layout
    sys->draw_2d_fit (renderer);
    sys->draw_2d (renderer);

    trace::Tracer tracer (sys.get ());

    // trace and draw rays from source
    tracer.get_params ().set_default_distribution (
        trace::Distribution (trace::MeridionalDist, 5));
    tracer.get_trace_result ().set_generated_save_state (*source);
    tracer.trace ();
    tracer.get_trace_result ().draw_2d (renderer);
    /* anchor end */
  }

  {
    /* anchor rayfan */
    io::RendererSvg renderer ("fan.svg", 640, 480, io::rgb_white);

    analysis::RayFan fan (sys);

    // select light source wavelens
    source->clear_spectrum ();
    source->add_spectral_line (light::SpectralLine::C);
    source->add_spectral_line (light::SpectralLine::e);
    source->add_spectral_line (light::SpectralLine::F);

    // get transverse aberration plot
    std::shared_ptr<data::Plot> fan_plot
        = fan.get_plot (analysis::RayFan::EntranceHeight,
                        analysis::RayFan::TransverseDistance);

    fan_plot->draw (renderer);
    /* anchor end */
  }
}
