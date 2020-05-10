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
#include <goptical/core/analysis/focus.hpp>
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

void
analysis_fan (std::shared_ptr<sys::System> &sys, const std::shared_ptr<sys::SourcePoint> &source_point);
void
analysis_spot (std::shared_ptr <sys::System> &sys, std::shared_ptr<sys::SourcePoint> &source_point);
void
layout (const std::shared_ptr <sys::System> &sys, const std::shared_ptr<sys::SourcePoint> &source_point);

static std::string
get_base_name (const std::string &name)
{
  auto pos = name.find_last_of ('.');
  if (pos == std::string::npos)
    return name;
  return name.substr (0, pos);
}

std::shared_ptr<sys::SourcePoint>
setup_point_source(std::shared_ptr< sys::System>& sys, double angleOfView, bool parallel)
{
  sys::SourceInfinityMode mode = sys::SourceAtInfinity;
  auto unit_vector = math::vector3_001;
  if (!parallel)
    {
      // Construct unit vector at an angle
      double z1 = cos (angleOfView);
      double y1 = sin (angleOfView);
      unit_vector = math::Vector3 (0, y1, z1);
    }
  auto source_point
    = std::make_shared<sys::SourcePoint>(mode, unit_vector);
  source_point->clear_spectrum ();
  source_point->add_spectral_line (light::SpectralLine::d);
  source_point->add_spectral_line (light::SpectralLine::C);
  source_point->add_spectral_line (light::SpectralLine::F);
  _goptical::sys::SystemBuilder builder;
  builder.add (sys, source_point);

  return source_point;
}

// TODO
// setup_source_rays() {
//  double z = 1e5;
//  double h = tan(angleOfView) * z;
//
//  sys::SourceRays  source_rays(math::Vector3(0, h, -z));
//
// sys::SourcePoint source_point(sys::SourceAtFiniteDistance,
//                              math::Vector3(0, h, -z));
// sys::SourcePoint source_point(sys::SourceAtFiniteDistance, math::Vector3(0,
// h, -z));
//  sys::SourcePoint source_point(sys::SourceAtInfinity, math::Vector3(0, 0,
//  1));

// add sources to system
//  sys.add(source_rays);
// configure sources
//    source_rays.add_chief_rays(sys);
//    source_rays.add_marginal_rays(sys, 14);

//  source_rays.clear_spectrum();
//  source_rays.add_spectral_line(light::SpectralLine::d);
//  source_rays.add_spectral_line(light::SpectralLine::C);
//  source_rays.add_spectral_line(light::SpectralLine::F);
//}

int
main (int argc, const char *argv[])
{
  //**********************************************************************
  // Optical system definition
    bool parallel_rays = true;
    bool refocus = false;
  if (argc < 2)
    {
      fprintf (stderr, "Please supply a data file\n");
      exit (1);
    }
  if (argc > 2)
    {
      parallel_rays = strchr(argv[2], 's') == nullptr;
      refocus = strchr(argv[2], 'f') != nullptr;
    }


  io::BClaffLensImporter importer;
  unsigned scenario = 0;

  std::string input_file (argv[1]);
  std::string output_base = get_base_name (input_file);
  std::string layout_file = output_base + "_layout.svg";
  std::string spot_file = output_base + "_spot.svg";
  std::string spot_intensity_file = output_base + "_spot_intensity.svg";
  std::string opd_fan_file = output_base + "_opd_fan.svg";
  std::string transverse_fan_file = output_base + "_transverse_fan.svg";
  std::string longitudinal_fan_file = output_base + "_longitudinal_fan.svg";

  double image_pos = importer.parseFile (argv[1], scenario);
  if (image_pos == 0.0)
    {
      exit (1);
    }

  auto sys = importer.get_system ();
  double angleOfView = importer.getAngleOfViewInRadians (scenario);

  /* anchor sources */
  auto source_point = setup_point_source (sys, angleOfView, parallel_rays);

  /* anchor seq */
  auto seq = std::make_shared<trace::Sequence>(*sys);
  sys->get_tracer_params ().set_sequential_mode (seq);

  std::cout << "system:" << std::endl << sys;
  std::cout << "sequence:" << std::endl << seq;
  /* anchor end */

  if (refocus) {
      /* anchor focus */
      analysis::Focus focus(sys);
      importer.get_image()->set_plane(focus.get_best_focus());
  }
  layout (sys, source_point);
  analysis_spot (sys, source_point);
  analysis_fan (sys, source_point);
  return 0;
}

//**********************************************************************
// Drawing rays and layout

void
layout (const std::shared_ptr<sys::System> &sys, const std::shared_ptr<sys::SourcePoint> &source_point)
{
  {
    /* anchor layout */
    io::RendererSvg renderer ("layout.svg", 800, 400);

#if 1
    // draw 2d system layout
    sys->draw_2d_fit (renderer);
    sys->draw_2d (renderer);
#else
    // draw 2d layout of lens only
    lens.draw_2d_fit (renderer);
    lens.draw_2d (renderer);
#endif

    trace::Tracer tracer (sys);

#if 0
    // trace and draw rays from rays source
    sys.enable_single<sys::Source>(source_rays);
    tracer.get_trace_result().set_generated_save_state(source_rays);
    tracer.trace();
    tracer.get_trace_result().draw_2d(renderer);
    /* anchor end */
#else
    // trace and draw rays from source
    tracer.get_params ().set_default_distribution (
      trace::Distribution (trace::MeridionalDist, 10));
    tracer.get_trace_result ().set_generated_save_state (*source_point);
    tracer.trace ();
    tracer.get_trace_result ().draw_2d (renderer);
#endif
  }
}
void
analysis_spot (std::shared_ptr<sys::System> &sys, std::shared_ptr<sys::SourcePoint> &source_point)
{
  /* anchor spot */
  sys->enable_single<sys::Source> (*source_point);

  sys->get_tracer_params ().set_default_distribution (
    trace::Distribution (trace::HexaPolarDist, 20));

  analysis::Spot spot (sys);

  /* anchor end */
  {
    /* anchor spot */
    io::RendererSvg renderer ("spot.svg", 300, 300, io::rgb_black);

    spot.draw_diagram (renderer);
    /* anchor end */
  }

  {
    /* anchor spot_plot */
    io::RendererSvg renderer ("spot_intensity.svg", 640, 480);

    std::shared_ptr<data::Plot> plot = spot.get_encircled_intensity_plot (50);

    plot->draw (renderer);
    /* anchor end */
  }
}
void
analysis_fan (std::shared_ptr<sys::System> &sys, const std::shared_ptr<sys::SourcePoint> &source_point)
{
  /* anchor opd_fan */
  sys->enable_single<sys::Source> (*source_point);

  analysis::RayFan fan (sys);

  /* anchor end */
  {
    /* anchor opd_fan */
    io::RendererSvg renderer ("opd_fan.svg", 640, 480);

    std::shared_ptr<data::Plot> fan_plot = fan.get_plot (analysis::RayFan::EntranceHeight,
					     analysis::RayFan::OpticalPathDiff);

    fan_plot->draw (renderer);

    /* anchor end */
  }

  {
    /* anchor transverse_fan */
    io::RendererSvg renderer ("transverse_fan.svg", 640, 480);

    std::shared_ptr<data::Plot> fan_plot
      = fan.get_plot (analysis::RayFan::EntranceHeight,
		      analysis::RayFan::TransverseDistance);

    fan_plot->draw (renderer);

    /* anchor end */
  }

  {
    /* anchor longitudinal_fan */
    io::RendererSvg renderer ("longitudinal_fan.svg", 640, 480);

    std::shared_ptr<data::Plot> fan_plot
      = fan.get_plot (analysis::RayFan::EntranceHeight,
		      analysis::RayFan::LongitudinalDistance);

    fan_plot->draw (renderer);

    /* anchor end */
  }
}
