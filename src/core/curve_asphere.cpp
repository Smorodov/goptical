#include <goptical/core/curve/curve_asphere.hpp>
#include <goptical/core/math/vector_pair.hpp>
#include <goptical/core/math/vector.hpp>

#include <cstdio>
#include <cmath>

namespace goptical {
  namespace curve {

    static inline double
    A (const goptical::curve::Asphere* S, double s2)
    {
      double s4 = s2 * s2;
      double s6 = s4 * s2;
      double s8 = s6 * s2;
      double s10 = s8 * s2;
      double s12 = s10 * s2;
      double s14 = s12 * s2;

      // We should add _A2*s2
      return S->_A4 * s4 + S->_A6 * s6 + S->_A8 * s8 + S->_A10 * s10 + S->_A12 * s12
	     + S->_A14 * s14;
    }

    static inline double
    A_ (const goptical::curve::Asphere* S, double s2)
    {
      double s4 = s2 * s2;
      double s6 = s4 * s2;
      double s8 = s6 * s2;
      double s10 = s8 * s2;
      double s12 = s10 * s2;

      // We should add 2*_A2
      return 4 * S->_A4 * s2 + 6 * S->_A6 * s4 + 8 * S->_A8 * s6 + 10 * S->_A10 * s8
	     + 12 * S->_A12 * s10 + 14 * S->_A14 * s12;
    }

    /* computes intersection using Feder's equations - code is taken from
     *
     */
    bool
    compute_intersection (Vector3 origin, Vector3 direction,
	       const goptical::curve::Asphere* S, Vector3& result)
    {
      /* Get length of ray between surfaces: NOTE: variable 't' was
       used by Feder as the vertex separation between previous
       surface and this surface. In the new scheme, in which rays
       are transformed to the coordinate system of the surface
       before tracing, 't' is _zero_. It is still present in this
       code to enable comparison with the Feder paper; the
       optimizing compiler will eliminate it from the
       expressions. */
      /* R->Q[0] is X, R->Q[1] is Y and R->Q[2] is Z */
      /* (X,Y,Z) is the vector along the ray to the surface */
      /* R->T[0] is x, R->T[1] is y and R->T[3] is z */
      /* (x,y,z) is the vector form of the vertex of the surface */
      /* Feder paper equation (1) */
      double t = 0;
      double e = (t * origin.x ()) - origin * direction;
      /* Feder paper equation (2) */
      double M_1x = origin.x () + e * direction.x () - t;
      /* Feder paper equation (3) */
      double M_1_2 = origin * origin - (e * e) + (t * t) - (2.0 * t * origin.x ());
      double r_1_2 = 1. / (S->_c * S->_c);
      if (M_1_2 > r_1_2)
	{
	  M_1_2 = r_1_2; /* SPECIAL RULE! 96-01-22 */
	}
      /* Feder paper equation (4) */
      double xi_1 = sqrt ((direction.x () * direction.x ())
			  - S->_c * (S->_c * M_1_2 - 2.0 * M_1x));
      if (isnan (xi_1))
	{ /* NaN! reject this ray! */
	  return false;
	}
      /* Feder paper equation (5) */
      double L = e + (S->_c * M_1_2 - 2.0 * M_1x) / (direction.x () + xi_1);

      /* Get intercept with new (spherical) surface: */
      double delta_length[3];
      for (int j = 0; j < 3; j++)
	delta_length[j] = -origin[j];
      result = origin + direction * L;
      result.x () = result.x () - t;
      /* Now R->T (result) has x1, y1, z1 */

      /* The ray has been traced to the osculating sphere with
	 curvature c1. Now we will iterate to get the intercept with
	 the nearby aspheric surface. Suppose the (rotationally
	 symmetric) aspheric is given by $$x = f(y,z)$$, and is a
	 function of $y^2 + z^2$ only. For a spherical surface, one
	 has $$x = r - (r^2 - s^2)^{1\over2}$$, where $s^2 = y^2 +
	 z^2$. For a general surface one may add deformation terms
	 to this expression and obtain $$x = c s^2 / (1 + (1 - c^2
	 s^2)^{1\over2})) + (A_2 s^2 + A_4 s^4 + ...) = f$$.  The
	 equation is expressed in this form in order to avoid
	 indeterminacy as c approaches zero, and in order to
	 represent surfaces that are nearly spherical. Near-spheres
	 cannot be handled well by a power series alone, especially
	 in the neighborhood of $s = 1 / c$.

	 In this implementation we include a term for the numerical
	 eccentricity so that we can trace any pure conic section
	 without using the $A_i$ terms. */
      enum
      {
	TOLMAX = 10
      };
      double tolerance = 1e-15;
      int j = 0;
      double delta = 0.0;
      do
	{
	  /* Get square of radius of intercept: */
	  /* Feder equation s^2 = y^2 + z^2, section E */
	  double s_2 = result.y () * result.y () + result.z () * result.z ();

	  /* Get the point on aspheric which is at the same radius as
	     the intercept of the ray. Then compute a tangent plane to
	     the aspheric at this point and find where it intersects
	     the ray.  This point will lie very close to the aspheric
	     surface.  The first step is to compute the x-coordinate
	     on the aspheric surface using $\overline{x}_0 = f(y_0,
	     z_0)$. */
	  /* (1 - c^2*s^2)^(1/2) - part of equation (12) */
	  double temp = sqrt (1.0 - S->_c * S->_c * s_2 * S->_k);
	  if (isnan (temp))
	    {
	      return false;
	    }
	  /* Feder equation (12) */
	  /* But using c*s^2/[1 + (1 - c^2*s^2)^(1/2)] + aspheric A_2*s^2 + A_4*s^4
	   * + ... */
	  double x_bar_0 = (S->_c * s_2) / (1.0 + temp) + A (S, s_2);
	  delta = fabs (result.x () - x_bar_0);

	  /* Get the direction numbers for the normal to the
	     aspheric: */
	  /* Feder equation (13), l */
	  Vector3 N;
	  N[0] = temp;
	  temp = S->_c + N[0] * A_ (S, s_2);
	  /* Feder equation (14), m */
	  N[1] = -result.y () * temp;
	  /* Feder equation (15), n */
	  N[2] = -result.z () * temp;

	  /* Get the distance from aspheric point to ray intercept */
	  double G_0 = N[0] * (x_bar_0 - result.x ()) / (direction * N);

	  /* and compute new estimate of intercept point: */
	  result = result + direction * G_0;
	}
      while ((delta > tolerance) && (++j < TOLMAX));
      if (j >= TOLMAX)
	{
	  printf ("rayTrace: delta=%g, reached %d iterations!?!\n", delta, j);
	  return false;
	}
      //  for (j = 0; j < 3; j++)
      //    delta_length[j] += R->T[j];
      //  delta_length[0] += t;
      //  index /= S->mu_1; /* effective_distance = dl*n */
      //  R->Length += (VECTOR_LENGTH(VC, delta_length) * fabs(index));

      return true;
    }

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

    bool Asphere::intersect(math::Vector3 &point, const math::VectorPair3 &ray) const {
	math::Vector3 result(0.,0, 0);
	math::Vector3 origin(ray.origin().z(), ray.origin().y(), ray.origin().x());
	math::Vector3 direction(ray.direction().z(), ray.direction().y(), ray.direction().x());
	compute_intersection(origin, direction, this, result);
	bool ok = Base::intersect(point, ray);
	if (fabs(point.x() - result.z()) > 1e-10 ||
	  fabs(point.y() - result.y()) > 1e-10 ||
	  fabs(point.z() - result.x()) > 1e-10)
	  {
	    printf ("%.16f %.16f, %.16f %.16f, %.16f %.16f\n", point.x (),
		    result.z (), point.y (), result.y (), point.z (),
		    result.x ());
	  }
	return ok;
    }

  } // namespace curve
} // namespace goptical