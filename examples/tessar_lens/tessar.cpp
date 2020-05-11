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

#include <goptical/core/math/Vector>

#include <goptical/core/material/Abbe>

#include <goptical/core/sys/System>
#include <goptical/core/sys/Lens>
#include <goptical/core/sys/Source>
#include <goptical/core/sys/SourceRays>
#include <goptical/core/sys/SourcePoint>
#include <goptical/core/sys/Image>
#include <goptical/core/sys/Stop>

#include <goptical/core/curve/Sphere>
#include <goptical/core/shape/Disk>

#include <goptical/core/trace/Tracer>
#include <goptical/core/trace/Result>
#include <goptical/core/trace/Distribution>
#include <goptical/core/trace/Sequence>
#include <goptical/core/trace/Params>

#include <goptical/core/light/SpectralLine>

#include <goptical/core/analysis/rayfan.hpp>
#include <goptical/core/analysis/spot.hpp>
#include <goptical/core/data/Plot>

#include <goptical/core/io/RendererSvg>
#include <goptical/core/io/Rgb>

using namespace goptical;

int main()
{
  //**********************************************************************
  // Optical system definition

  auto sys = std::make_shared<sys::System>();

  /* anchor lens */
  auto lens = std::make_shared<sys::Lens>(math::Vector3(0, 0, 0));

  //               roc,            ap.radius, thickness,

  _goptical::sys::LensBuilder lensBuilder;
  lensBuilder.add_surface(lens, 1/0.031186861,  14.934638, 4.627804137,
                   std::make_shared<material::AbbeVd>(1.607170, 59.5002));

  lensBuilder.add_surface(lens, 0,              14.934638, 5.417429465);

  lensBuilder.add_surface(lens, 1/-0.014065441, 12.766446, 3.728230979,
                   std::make_shared<material::AbbeVd>(1.575960, 41.2999));

  lensBuilder.add_surface(lens, 1/0.034678487,  11.918098, 4.417903733);

  lensBuilder.add_stop(lens, 12.066273, 2.288913925);

  lensBuilder.add_surface(lens, 0,              12.372318, 1.499288597,
                   std::make_shared<material::AbbeVd>(1.526480, 51.4000));

  lensBuilder.add_surface(lens, 1/0.035104369,  14.642815, 7.996205852,
                   std::make_shared<material::AbbeVd>(1.623770, 56.8998));

  lensBuilder.add_surface(lens, 1/-0.021187519, 14.642815, 85.243965130);

  _goptical::sys::SystemBuilder builder;
  builder.add(sys, lens);
  /* anchor end */

  auto image = std::make_shared<sys::Image>(math::Vector3(0, 0, 125.596), 5);
  builder.add(sys, image);

  /* anchor sources */
  auto source_rays = std::make_shared<sys::SourceRays>(math::Vector3(0, 27.5, -1000));

  auto source_point = std::make_shared<sys::SourcePoint>(sys::SourceAtFiniteDistance,
                                math::Vector3(0, 27.5, -1000));

  // add sources to system
  builder.add(sys, source_rays);
  builder.add(sys, source_point);

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

