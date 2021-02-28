/*

      This file is part of the Goptical library.

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

#include <goptical/core/sys/group.hpp>
#include <goptical/core/sys/image.hpp>
#include <goptical/core/sys/mirror.hpp>
#include <goptical/core/sys/source_point.hpp>
#include <goptical/core/sys/stop.hpp>
#include <goptical/core/sys/system.hpp>

#include <goptical/core/shape/base.hpp>
#include <goptical/core/shape/disk.hpp>
#include <goptical/core/shape/regular_polygon.hpp>
#include <goptical/core/shape/ring.hpp>

#include <goptical/core/curve/base.hpp>
#include <goptical/core/curve/composer.hpp>
#include <goptical/core/curve/conic.hpp>

#include <goptical/core/trace/distribution.hpp>
#include <goptical/core/trace/params.hpp>
#include <goptical/core/trace/result.hpp>
#include <goptical/core/trace/tracer.hpp>

#include <goptical/core/io/renderer_svg.hpp>
#include <goptical/core/io/renderer_viewport.hpp>

#include <goptical/core/math/vector.hpp>
#include <goptical/core/math/vector_pair.hpp>

#include <goptical/core/error.hpp>

using namespace goptical;

/* anchor hexseg1 */
class HexSegMirror : public sys::Group
{
public:
  HexSegMirror (const math::VectorPair3 &pos,
                const std::shared_ptr<curve::Base> &curve,
                const std::shared_ptr<shape::Base> &shape, double seg_radius,
                double separation)
      : sys::Group (pos)
  {
    /* anchor hexseg2 */
    if (seg_radius > separation)
      throw (Error ("overlapping segments"));

    // sqrt(3)/2
    static const double sqrt_3_2 = 0.86602540378443864676;

    // hexagonal tessellation
    int x_count = ceil (shape->max_radius () / (separation * 1.5));
    int y_count = ceil (shape->max_radius () / (separation * 2 * sqrt_3_2));

    for (int x = -x_count; x <= x_count; x++)
      {
        for (int y = -y_count; y <= y_count; y++)
          {
            // find segment mirror 2d position
            double yoffset = x % 2 ? separation * sqrt_3_2 : 0;
            math::Vector2 p (x * separation * 1.5,
                             yoffset + y * separation * 2 * sqrt_3_2);
            /* anchor hexseg3 */
            // skip if segment center is outside main shape
            if (!shape->inside (p))
              continue;
            /* anchor hexseg4 */
            // find curve z offset at segment center to shift both
            // curve and segment in opposite directions.
            double z_offset = curve->sagitta (p);

            // create a composer curve for this segment and use it to translate
            // main curve
            std::shared_ptr<curve::Composer> seg_curve
                = std::make_shared<curve::Composer> ();

            seg_curve->add_curve (curve).xy_translate (-p).z_offset (
                -z_offset);
            /* anchor hexseg5 */
            // create a segment mirror with hexagonal shape and translated
            // curve
            std::shared_ptr<sys::Mirror> seg = std::make_shared<sys::Mirror> (
                math::Vector3 (p, z_offset), seg_curve,
                std::make_shared<shape::RegularPolygon> (seg_radius, 6));

            // attach the new segment to our group component
            add (seg);
            /* anchor hexseg6 */
            // keep a pointer to this new segment
            _segments.push_back (seg);
          }
      }
  }

  size_t
  get_segments_count () const
  {
    return _segments.size ();
  }

  std::shared_ptr<sys::Mirror>
  get_segment (size_t i)
  {
    return _segments.at (i);
  }

private:
  std::vector<std::shared_ptr<sys::Mirror> > _segments;
};
/* anchor end */

int
main ()
{

  // system focal length                           2400
  // Back focal length                              100

  // Primary mirror diameter                        300
  // Primary mirror focal length                    800
  // Primary mirror conic constant              -1.0869

  // Distance secondary from system focus           675
  // Distance secondary from prime focus            225
  // Distance secondary from primary                575
  // Radius of curvature of secondary mirror        675
  // Secondary mirror conic constant            -5.0434

  /* anchor rc */
  auto sys = std::make_shared<sys::System> ();

  // Ring shaped segmented mirror with conic curve
  auto primary = std::make_shared<HexSegMirror> (
      math::Vector3 (0, 0, 800),
      std::make_shared<curve::Conic> (-1600, -1.0869),
      std::make_shared<shape::Ring> (300, 85), 28, 30);
  sys->add (primary);

  auto secondary = std::make_shared<sys::Mirror> (
      math::VectorPair3 (0, 0, 225, 0, 0, -1), 675, -5.0434, 100);
  sys->add (secondary);

  auto image
      = std::make_shared<sys::Image> (math::VectorPair3 (0, 0, 900), 15);
  sys->add (image);

  auto stop = std::make_shared<sys::Stop> (math::vector3_0, 300);
  sys->add (stop);
  sys->set_entrance_pupil (stop);

  auto source = std::make_shared<sys::SourcePoint> (sys::SourceAtInfinity,
                                                    math::vector3_001);
  sys->add (source);
  /* anchor end */

  //  primary.get_segment(5).rotate(0.001, 0, 0);

  /* anchor layout */
  trace::Tracer tracer (sys.get ());

  // trace rays through the system
  tracer.get_trace_result ().set_generated_save_state (*source);
  tracer.trace ();

#if 0
  // FIXME following doesn't seem to work
  // Original code used a different renderer
  io::RendererSvg svg_renderer ("layout.svg", 640, 480);
  io::RendererViewport &renderer = svg_renderer;

  renderer.set_page_layout (1, 1);

  // 3d system layout on 1st sub-page
  renderer.set_page (0);
  renderer.set_perspective ();

  sys->draw_3d_fit (renderer, 0);
  sys->draw_3d (renderer);

  tracer.get_trace_result ().draw_3d (renderer);
  /* anchor end */
#else
  fprintf (stderr, "Sorry not yet implemented\n");
#endif

  return 0;
}
