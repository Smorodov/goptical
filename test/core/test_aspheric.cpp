#include <goptical/core/curve/curve_asphere.hpp>
#include <goptical/core/curve/sphere.hpp>
#include <goptical/core/math/vector.hpp>
#include <goptical/core/math/vector_pair.hpp>

#include <memory>
#include <cstdio>



int
main (int argc, const char *argv[])
{
   auto surface = std::make_shared<goptical::curve::Asphere>(1.0/0.25284872, 1.0, -0.005,
   0.00001, -0.0000005, 0);

//  auto surface = std::make_shared<goptical::curve::Sphere> (1.0 / 0.25284872);
  Vector3 pos_dir (0.0, 0.1736, 0.98481625);
  Vector3 origin (1.48, 0.0, 0.0);
  VectorPair3 ray (origin, pos_dir);
  Vector3 point (0, 0, 0);
  surface->intersect (point, ray);
  printf("%.16f, %.16f, %.16f\n", point.x(), point.y(), point.z());

  Vector3 pos_dir2 (0.98481625, 0.1736, 0.0);
  Vector3 origin2 (0, 0.0, 1.48);
  Vector3 point2 (0,0,0);
  goptical::curve::compute_intersection(origin2, pos_dir2, surface.get(), point2);
  printf("%.16f, %.16f, %.16f\n", point2.x(), point2.y(), point2.z());

}