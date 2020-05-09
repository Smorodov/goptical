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

#include <fstream>
#include <iostream>

#include <goptical/core/math/Vector>

#include <goptical/core/material/Abbe>

#include <goptical/core/sys/Image>
#include <goptical/core/sys/Lens>
#include <goptical/core/sys/Source>
#include <goptical/core/sys/SourcePoint>
#include <goptical/core/sys/SourceRays>
#include <goptical/core/sys/Stop>
#include <goptical/core/sys/System>

#include <goptical/core/curve/Rotational>
#include <goptical/core/curve/Sphere>
#include <goptical/core/shape/Disk>

#include <goptical/core/trace/Distribution>
#include <goptical/core/trace/Params>
#include <goptical/core/trace/Result>
#include <goptical/core/trace/Sequence>
#include <goptical/core/trace/Tracer>

#include <goptical/core/light/SpectralLine>

#include <goptical/core/analysis/rayfan.hpp>
#include <goptical/core/analysis/spot.hpp>
#include <goptical/core/data/Plot>

#include <goptical/core/io/RendererSvg>
#include <goptical/core/io/Rgb>

#include <goptical/core/math/Transform>

#include <goptical/core/io/import_bclaff.hpp>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iterator>
#include <string>
#include <vector>
#include <memory>

using namespace goptical;

int main(int argc, const char *argv[]) {
  //**********************************************************************
  // Optical system definition

  if (argc != 2) {
    fprintf(stderr, "Please supply a data file\n");
    exit(1);
  }

  sys::System sys;
  io::BClaffLensImporter importer;
  unsigned scenario = 0;

  double image_pos = importer.parseFile(argv[1], sys, scenario);
  if (image_pos == 0.0) {
    exit(1);
  }

  double angleOfView = importer.getAngleOfViewInRadians(scenario);

  /* anchor sources */
    //sys::SourceRays  source_rays(math::Vector3(0, h, -z));

    //sys::SourcePoint source_point(sys::SourceAtFiniteDistance,
    //                              math::Vector3(0, h, -z));

  double z1 = cos(angleOfView);
  double y1 = sin(angleOfView);
  std::cerr << "z1 = " << z1 << "\n";
  std::cerr << "y1 = " << y1 << "\n";
//    sys::SourcePoint source_point(sys::SourceAtInfinity,
//                                  math::Vector3(0, y1, z1));

  //sys::SourcePoint source_point(sys::SourceAtFiniteDistance, math::Vector3(0, h, -z));
  sys::SourcePoint source_point(sys::SourceAtInfinity, math::Vector3(0, 0, 1));

  // add sources to system
  //sys.add(source_rays);
  sys.add(source_point);

  // configure sources
    //source_rays.add_chief_rays(sys);
    //source_rays.add_marginal_rays(sys, 14);

  source_point.clear_spectrum();
  source_point.add_spectral_line(light::SpectralLine::d);
  source_point.add_spectral_line(light::SpectralLine::C);
  source_point.add_spectral_line(light::SpectralLine::F);

  //source_rays.clear_spectrum();
  //source_rays.add_spectral_line(light::SpectralLine::d);

  /* anchor end */

  /* anchor seq */
  trace::Sequence seq(sys);

  //  sys.get_tracer_params().set_sequential_mode(seq);
  std::cout << "system:" << std::endl << sys;
  std::cout << "sequence:" << std::endl << seq;
  /* anchor end */

  //**********************************************************************
  // Drawing rays and layout

  {
    /* anchor layout */
    io::RendererSvg renderer("layout.svg", 800, 400);

#if 1
    // draw 2d system layout
    sys.draw_2d_fit(renderer);
    sys.draw_2d(renderer);
#else
    // draw 2d layout of lens only
    lens.draw_2d_fit(renderer);
    lens.draw_2d(renderer);
#endif

    trace::Tracer tracer(sys);

#if 0
    // trace and draw rays from rays source
    sys.enable_single<sys::Source>(source_rays);
    tracer.get_trace_result().set_generated_save_state(source_rays);

    tracer.trace();
    tracer.get_trace_result().draw_2d(renderer);
    /* anchor end */
#else
    // trace and draw rays from source
    tracer.get_params().set_default_distribution(
        trace::Distribution(trace::MeridionalDist, 10));
    tracer.get_trace_result().set_generated_save_state(source_point);
    tracer.trace();
    tracer.get_trace_result().draw_2d(renderer);
#endif
  }

  {
    /* anchor spot */
    sys.enable_single<sys::Source>(source_point);

    sys.get_tracer_params().set_default_distribution(
        trace::Distribution(trace::HexaPolarDist, 20));

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

      ref<data::Plot> plot = spot.get_encircled_intensity_plot(50);

      plot->draw(renderer);
      /* anchor end */
    }
  }

  {
    /* anchor opd_fan */
    sys.enable_single<sys::Source>(source_point);

    analysis::RayFan fan(sys);

    /* anchor end */
    {
      /* anchor opd_fan */
      io::RendererSvg renderer("opd_fan.svg", 640, 480);

      ref<data::Plot> fan_plot = fan.get_plot(
          analysis::RayFan::EntranceHeight, analysis::RayFan::OpticalPathDiff);

      fan_plot->draw(renderer);

      /* anchor end */
    }

    {
      /* anchor transverse_fan */
      io::RendererSvg renderer("transverse_fan.svg", 640, 480);

      ref<data::Plot> fan_plot =
          fan.get_plot(analysis::RayFan::EntranceHeight,
                       analysis::RayFan::TransverseDistance);

      fan_plot->draw(renderer);

      /* anchor end */
    }

    {
      /* anchor longitudinal_fan */
      io::RendererSvg renderer("longitudinal_fan.svg", 640, 480);

      ref<data::Plot> fan_plot =
          fan.get_plot(analysis::RayFan::EntranceHeight,
                       analysis::RayFan::LongitudinalDistance);

      fan_plot->draw(renderer);

      /* anchor end */
    }
  }
  return 0;
}
