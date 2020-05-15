#ifndef GOPTICAL_CURVE_ASPHERE_HPP
#define GOPTICAL_CURVE_ASPHERE_HPP

#include <goptical/core/curve/rotational.hpp>

namespace goptical {
  namespace curve {

    class Asphere : public goptical::curve::Rotational
    {
    public:
      Asphere (double r, double k, double A4, double A6, double A8, double A10,
	       double A12 = 0.0, double A14 = 0.0)
	: _r (r), _c(1.0/r), _k (k), _A4 (A4), _A6 (A6), _A8 (A8), _A10 (A10), _A12 (A12),
	  _A14 (A14)
      {}

      double sagitta (double y) const override;
      virtual bool intersect(math::Vector3 &point, const math::VectorPair3 &ray) const;
      virtual void normal(math::Vector3 &normal, const math::Vector3 &point) const;
    public:
      double _r;
      double _c;
      double _k;
      double _A4;
      double _A6;
      double _A8;
      double _A10;
      double _A12;
      double _A14;
    };

    bool
    compute_intersection (Vector3 origin, Vector3 direction,
			  const goptical::curve::Asphere* S, Vector3& result);

  } // namespace curve
} // namespace goptical

#endif // GOPTICAL_CURVE_ASPHERE_HPP
