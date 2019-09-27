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
#include <goptical/core/curve/Rotational>

#include <goptical/core/shape/Disk>


#include <goptical/core/trace/Tracer>
#include <goptical/core/trace/Result>
#include <goptical/core/trace/Distribution>
#include <goptical/core/trace/Sequence>
#include <goptical/core/trace/Params>

#include <goptical/core/light/SpectralLine>

#include <goptical/core/analysis/RayFan>
#include <goptical/core/analysis/Spot>
#include <goptical/core/data/Plot>

#include <goptical/core/io/RendererSvg>
#include <goptical/core/io/Rgb>

using namespace goptical;

class AsphericCurve : public curve::Rotational {

public:
    AsphericCurve(double r, double k, double A4, double A6, double A8, double A10 = 0.0, double A12 = 0.0, double A14 = 0.0):
        _r(r), _k(k), _A4(A4), _A6(A6), _A8(A8), _A10(A10), _A12(A12), _A14(A14)
    {
    }

    double sagitta(double y) const override {
        double yy = y*y;
        double y4 = yy*yy;
        double y6 = y4*yy;
        double y8 = y6*yy;
        double y10 = y8*yy;
        double y12 = y10*yy;
        double y14 = y12*yy;
        double rr = _r*_r;
        double z = (yy / _r) / (1.0 + pow(1.0 - _k * (yy/rr), 0.5))
            + _A4*y4 + _A6*y6 + _A8*y8 + _A10*y10 + _A12*y12 + _A14*y14;
        return z;
    }

private:
    double _r;
    double _k;
    double _A3;
    double _A4;
    double _A6;
    double _A8;
    double _A10;
    double _A12;
    double _A14;
};

int main()
{
  //**********************************************************************
  // Optical system definition

  sys::system   sys;

  /* anchor lens */
  sys::Lens     lens(math::Vector3(0, 0, 0));

  //               roc,            ap.radius, thickness,

  lens.add_surface(
    ref<AsphericCurve>::create(52.8577, 0.5721, 1.10084e-07, 6.21998e-10, -4.25694e-13),
    ref<shape::Disk>::create(24.765), 6.0, ref<material::AbbeVd>::create(1.744430, 49.53));

  lens.add_surface(229.3475,              23.61, 0.1);

  lens.add_surface(40.3738, 19.79, 6.0,
                   ref<material::AbbeVd>::create(1.7550, 52.34));

  lens.add_surface(354.9744, 19.79, 1.5,
                   ref<material::AbbeVd>::create(1.48749, 70.31));

  lens.add_surface(42.4134,  17.53, 4.1038);

  lens.add_surface(290.8467, 17.01, 1.5,
                  ref<material::AbbeVd>::create(1.688930, 31.16));

  lens.add_surface(31.6359,  16.54, 6.0);


  lens.add_stop   (                14.1, 6.0);

  lens.add_surface(-30.7873,  15.02, 1.7,
                   ref<material::AbbeVd>::create(1.72825, 28.46));

  lens.add_surface(35.1427,   16.85, 7.0,
                   ref<material::AbbeVd>::create(1.883, 40.66));

  lens.add_surface(-131.1407,  16.85, 0.1);

  lens.add_surface(118.7661, 17.06, 6.0,
                   ref<material::AbbeVd>::create(1.883, 40.66));

  lens.add_surface(-44.2318, 17.06, 1.5,
                   ref<material::AbbeVd>::create(1.53172, 48.78));

  lens.add_surface(44.2683, 17.06, 6.0,
                   ref<material::AbbeVd>::create(1.7443, 49.53));

  lens.add_surface(
    ref<AsphericCurve>::create(-77.2943, 14.1597, 8.65514e-06, 4.1594e-09, 1.25812e-11, 1.22728e-14),
    ref<shape::Disk>::create(17.06), 38.7);

  auto& s1 = lens.get_surface(0);

  double image_pos = 6.0 + 0.1 + 6.0 + 1.5 + 4.1038 + 1.5
    + 6.0 + 6.0 + 1.7 + 7.0 + 0.1 + 6.0 + 1.5 + 6.0 + 38.7;
  printf("Image pos at %f\n", image_pos);

  sys.add(lens);
  /* anchor end */

  sys::Image      image(math::Vector3(0, 0, image_pos), 42.42);
  sys.add(image);

  sys.set_entrance_pupil(s1);

  /* anchor sources */
//  sys::SourceRays  source_rays(math::Vector3(0, 127.5, -10000));

//  sys::SourcePoint source_point(sys::SourceAtFiniteDistance,
//                                math::Vector3(0, 27.5, -10000));

//  sys::SourcePoint source_point(sys::SourceAtInfinity,
//                                math::Vector3(0, 0, 1));
  sys::SourcePoint source_point(sys::SourceAtInfinity,
                                  math::Vector3(0, .2, 1));

  // add sources to system
//  sys.add(source_rays);
  sys.add(source_point);

  // configure sources
//  source_rays.add_chief_rays(sys);
//  source_rays.add_marginal_rays(sys, 20.0);

  source_point.clear_spectrum();
  source_point.add_spectral_line(light::SpectralLine::d);
  /* anchor end */

  /* anchor seq */
  trace::Sequence seq(sys);

  //sys.get_tracer_params().set_sequential_mode(seq);
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

    trace::tracer tracer(sys);
#if 0
    // trace and draw rays from rays source
    sys.enable_single<sys::Source>(source_rays);
    tracer.get_trace_result().set_generated_save_state(source_rays);

    tracer.trace();
    tracer.get_trace_result().draw_2d(renderer);
#else
    // trace and draw rays from source
    tracer.get_params().set_default_distribution(
        trace::Distribution(trace::MeridionalDist, 10));
    tracer.get_trace_result().set_generated_save_state(source_point);
    tracer.trace();
    tracer.get_trace_result().draw_2d(renderer);
#endif
    /* anchor end */
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

//  {
//    /* anchor spot */
//    io::RendererSvg            renderer("spot2.svg",        300 * 3, 300 * 2, io::rgb_black);
//
//    renderer.set_margin_ratio(.35, .25, .1, .1);
//    renderer.set_page_layout(3, 2);
//
//    for (int i = 0; i < 6; i++)
//    {
//      analysis::Spot spot(sys);
//
//      renderer.set_page(i);
//      spot.draw_diagram(renderer);
//
//      source_point.rotate(0, .1, 0);
//    }
//  }


  {
    /* anchor opd_fan */
    sys.enable_single<sys::Source>(source_point);

    analysis::RayFan fan(sys);

    /* anchor end */
    {
    /* anchor opd_fan */
      io::RendererSvg renderer("opd_fan.svg", 640, 480);

      ref<data::Plot> fan_plot = fan.get_plot(analysis::RayFan::EntranceHeight,
                                              analysis::RayFan::OpticalPathDiff);

      fan_plot->draw(renderer);

    /* anchor end */
    }

    {
    /* anchor transverse_fan */
      io::RendererSvg renderer("transverse_fan.svg", 640, 480);

      ref<data::Plot> fan_plot = fan.get_plot(analysis::RayFan::EntranceHeight,
                                              analysis::RayFan::TransverseDistance);

      fan_plot->draw(renderer);

    /* anchor end */
    }

    {
    /* anchor longitudinal_fan */
      io::RendererSvg renderer("longitudinal_fan.svg", 640, 480);

      ref<data::Plot> fan_plot = fan.get_plot(analysis::RayFan::EntranceHeight,
                                              analysis::RayFan::LongitudinalDistance);

      fan_plot->draw(renderer);

    /* anchor end */
    }
  }

  return 0;
}

