#include <goptical/core/curve/curve_asphere.hpp>

namespace goptical {
  namespace curve {

    double Asphere::sagitta (double y) const
    {
      double yy = y * y;
      double y4 = yy * yy;
      double y6 = y4 * yy;
      double y8 = y6 * yy;
      double y10 = y8 * yy;
      double y12 = y10 * yy;
      double y14 = y12 * yy;
      double rr = _r * _r;
      double z = (yy / _r) / (1.0 + pow (1.0 - _k * (yy / rr), 0.5)) + _A4 * y4
		 + _A6 * y6 + _A8 * y8 + _A10 * y10 + _A12 * y12 + _A14 * y14;
      return z;
    }

  } // namespace curve
} // namespace goptical