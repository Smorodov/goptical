#include <goptical/core/curve/curve_asphere.hpp>
#include <goptical/core/math/vector_pair.hpp>
#include <goptical/core/math/vector.hpp>

#include <cstdio>
#include <cmath>

namespace goptical {
  namespace curve {

    static inline double A (const goptical::curve::Asphere *S, double s2)
    {
      double s4 = s2 * s2;
      double s6 = s4 * s2;
      double s8 = s6 * s2;
      double s10 = s8 * s2;
      double s12 = s10 * s2;
      double s14 = s12 * s2;

      // We should add _A2*s2
      return S->_A4 * s4 + S->_A6 * s6 + S->_A8 * s8 + S->_A10 * s10
	     + S->_A12 * s12 + S->_A14 * s14;
    }

    static inline double A_ (const goptical::curve::Asphere *S, double s2)
    {
      double s4 = s2 * s2;
      double s6 = s4 * s2;
      double s8 = s6 * s2;
      double s10 = s8 * s2;
      double s12 = s10 * s2;

      // We should add 2*_A2
      return 4 * S->_A4 * s2 + 6 * S->_A6 * s4 + 8 * S->_A8 * s6
	     + 10 * S->_A10 * s8 + 12 * S->_A12 * s10 + 14 * S->_A14 * s12;
    }

    static inline double deform_dz_ds (const goptical::curve::Asphere *S, double s)
    {
      double s2 = s * s;
      double s3 = s2 * s;
      double s5 = s3 * s2;
      double s7 = s5 * s2;
      double s9 = s7 * s2;
      double s11 = s9 * s2;
      double s13 = s11 * s2;

      // We should add 2*_A2
      return 4 * S->_A4 * s3 + 6 * S->_A6 * s5 + 8 * S->_A8 * s7
	     + 10 * S->_A10 * s9 + 12 * S->_A12 * s11 + 14 * S->_A14 * s13;
    }

    /* computes intersection using Feder's equations - code is taken from
     * https://github.com/dibyendumajumdar/ray.
     * Note that Feder's paper uses x-axis rather than z-axis as the
     * optical axis, so below we switch from x to z.
     */
    bool compute_intersection (Vector3 origin, Vector3 direction,
			       const goptical::curve::Asphere *S,
			       Vector3 &result)
    {
      /* NOTE: variable 't' was
       used by Feder as the vertex separation between previous
       surface and this surface. In the new scheme, in which rays
       are transformed to the coordinate system of the surface
       before tracing, 't' is _zero_. It is still present in this
       code to enable comparison with the Feder paper; the
       optimizing compiler will eliminate it from the
       expressions. */
      /* direction (X,Y,Z) is the vector along the ray to the surface */
      /* origin (x,y,z) is the vector form of the vertex of the surface */
      /* Feder paper equation (1) */
      double t = 0;
      double e = (t * origin.z ()) - origin * direction;
      /* Feder paper equation (2) */
      double M_1x = origin.z () + e * direction.z () - t;
      /* Feder paper equation (3) */
      double M_1_2
	= origin * origin - (e * e) + (t * t) - (2.0 * t * origin.z ());
      double r_1_2 = 1. / (S->_c * S->_c);
      if (M_1_2 > r_1_2)
	{
	  M_1_2 = r_1_2; /* SPECIAL RULE! 96-01-22 */
	}
      /* Feder paper equation (4) */
      double xi_1 = sqrt ((direction.z () * direction.z ())
			  - S->_c * (S->_c * M_1_2 - 2.0 * M_1x));
      if (isnan (xi_1))
	{ /* NaN! reject this ray! */
	  return false;
	}
      /* Feder paper equation (5) */
      double L = e + (S->_c * M_1_2 - 2.0 * M_1x) / (direction.z () + xi_1);

      /* Get intercept with new (spherical) surface: */
      double delta_length[3];
      for (int j = 0; j < 3; j++)
	delta_length[j] = -origin[j];
      result = origin + direction * L;
      result.z () = result.z () - t;
      /* Now (result) has x1, y1, z1 */

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
	  /* Feder equation s^2 = x^2 + y^2, section E */
	  double s_2 = result.y () * result.y () + result.x () * result.x ();

	  /* Get the point on aspheric which is at the same radius as
	     the intercept of the ray. Then compute a tangent plane to
	     the aspheric at this point and find where it intersects
	     the ray.  This point will lie very close to the aspheric
	     surface.  The first step is to compute the z-coordinate
	     on the aspheric surface using $\overline{z}_0 = f(x_0,
	     y_0)$. */
	  /* (1 - k*c^2*s^2)^(1/2) - part of equation (12) */
	  double temp = sqrt (1.0 - S->_c * S->_c * s_2 * S->_k);
	  if (isnan (temp) || (1.0+temp) == 0.0)
	    {
	      return false;
	    }
	  /* Feder equation (12) */
	  /* But using c*s^2/[1 + (1 - k*c^2*s^2)^(1/2)] + aspheric A_2*s^2 +
	   * A_4*s^4 + ... */
	  double x_bar_0 = (S->_c * s_2) / (1.0 + temp) + A (S, s_2);
	  delta = fabs (result.z () - x_bar_0);

	  /* Get the direction numbers for the normal to the
	     aspheric: */
	  /* Feder equation (13), l */
	  Vector3 N;
	  N.z () = temp;
	  temp = S->_c + N.z () * A_ (S, s_2);
	  /* Feder equation (14), m */
	  N.y () = -result.y () * temp;
	  /* Feder equation (15), n */
	  N.x () = -result.x () * temp;

	  /* Get the distance from aspheric point to ray intercept */
	  double G_0 = N.z () * (x_bar_0 - result.z ()) / (direction * N);

	  /* and compute new estimate of intercept point: */
	  result = result + direction * G_0;
	}
      while ((delta > tolerance) && (++j < TOLMAX));
      if (j >= TOLMAX)
	{
	  printf ("rayTrace: delta=%g, reached %d iterations!?!\n", delta, j);
	  return false;
	}
      return true;
    }

    bool compute_normal (const goptical::curve::Asphere *S,
			 const Vector3 &point, Vector3 &N)
    {
      /* General ray tracing procedure - Spencer and Murty */
      /* See eq 18, 19 */
      /* Also same as p632 Feder - but z axis swapped with x */
      double s_2 = point.y () * point.y () + point.x () * point.x ();
      double temp = sqrt (1.0 - S->_c * S->_c * s_2 * S->_k);
      if (temp == 0.0)
	{
	  N.x () = 0;
	  N.y () = 0;
	  N.z () = 1.0;
	  return true; // See curve_rotational.cpp - we mimic
	}
      if (isnan (temp))
	{
	  return false;
	}
      double E = S->_c / temp
		 + A_ (S, s_2); // eq 19
      N.y () = -point.y () * E; // eq 18
      N.x () = -point.x () * E; // eq 18
      N.z () = 1.0;		// eq 18
      // Following is from Goptical - tbc
      N.normalize ();
      return true;
    }

    /** Compute z at s^2, where s^2 = x^2 + y^2 */
    static double compute_Z (const Asphere *surface, double s2)
    {
      /* Our formula is:
       * z = f(s) = c*s^2/(1 + (1 - c^2*k*s^2)^(1/2)) + A4*s^4 + A6*s^6 + A8*s^8 + A10*s^10 + + A12*s^12 + A14*s^14
       *
       * where s = (x^2 + y^2)^(1/2)
       */
      double c = surface->_c; /* curvature = 1/radius */
      double c2 = c * c;
      double K = surface->_k;
      double l = sqrt (1 - s2 * K * c2);
      double temp = 1 + l;
      if (temp == 0.0)
	{
	  // division by zero, really an error
	  return 0;
	}
      return c * s2 / temp + A (surface, s2);
    }

    /** Compute z at x,y */
    static double compute_Z (const Asphere *surface, const math::Vector2 &xy)
    {
      /* Our formula is:
       * z = f(s) = c*s^2/(1 + (1 - c^2*k*s^2)^(1/2)) + A4*s^4 + A6*s^6 + A8*s^8 + A10*s^10 + + A12*s^12 + A14*s^14
       *
       * where s = (x^2 + y^2)^(1/2)
       */
      double s2 = xy.x () * xy.x () + xy.y () * xy.y ();
      return compute_Z (surface, s2);
    }

    /** Compute dz/ds. For the equation see next function below. */
    static double compute_derivative (const Asphere *surface, double s)
    {
      double s2 = s * s;
      double c = surface->_c; /* curvature = 1/radius */
      double c2 = c * c;
      double K = surface->_k;
      double l = sqrt (1 - s2 * K * c2);
      if (l == 0.0)
	{
	  // division by zero, really an error
	  return 0;
	}
      return (c * s) / l + deform_dz_ds (surface, s);
    }

    /** Compute dz/dx and dz/dy at x,y */
    static void compute_derivative (const Asphere *surface,
				    const math::Vector2 &xy,
				    math::Vector2 &dxdy)
    {
      /*
       * Let s^2 = x^2 + y^2
       * and,
       * z = f(s) = c*s^2/(1 + (1 - c^2*k*s^2)^(1/2)) + A4*s^4 + A6*s^6 + A8*s^8 + A10*s^10 + + A12*s^12 + A14*s^14
       *
       * Then,
       * dz/dx = dz/ds * ds/dx
       *
       * Now,
       * dz/ds = c*s/(1 - c^2*k*s^2)^(1/2) + 4*A4*s^3 + 6*A6*s^5 + 8*A8*s^7 + 10*A10*s^9 + 12*A12*s^11 + 14*A14*s^13
       * and,
       * ds/dx = x/s
       *
       * using
       * E = dz/ds * 1/s = c/(1 - c^2*k*s^2)^(1/2) + 4*A4*s^2 + 6*A6*s^4 + 8*A8*s^6 + 10*A10*s^8 + 12*A12*s^10 + 14*A14*s^12
       * dz/dx = x*E
       * and
       * dz/dy = y*E
       */
      double s = sqrt (xy.x () * xy.x () + xy.y () * xy.y ());
      double E = compute_derivative (surface, s) / s;
      dxdy.x () = xy.x () * E;
      dxdy.y () = xy.y () * E;
    }

    double Asphere::sagitta (double s) const
    {
      double s2 = s * s;
      double y4 = s2 * s2;
      double y6 = y4 * s2;
      double y8 = y6 * s2;
      double y10 = y8 * s2;
      double y12 = y10 * s2;
      double y14 = y12 * s2;
      double rr = _r * _r;
      double z = (s2 / _r) / (1.0 + pow (1.0 - _k * (s2 / rr), 0.5)) + _A4 * y4
		 + _A6 * y6 + _A8 * y8 + _A10 * y10 + _A12 * y12 + _A14 * y14;
      double z1 = compute_Z (this, s2);
      if (fabs (z - z1) > 1e-14)
	{
	  printf ("Error computing z - expected %.16f got %.16f\n", z, z1);
	}
      return z;
    }

    double Asphere::derivative (double r) const
    {
      double value = Rotational::derivative (r);
      double value2 = compute_derivative (this, r);
      if (fabs (value - value2) > 1e-10)
	{
	  printf ("Error computing derivative(s) - expected %.16f got %.16f\n",
		  value, value2);
	}
      return value2;
    }

    double Asphere::sagitta (const math::Vector2 &xy) const
    {
      double sag1 = sagitta (xy.len ());
      double sag2 = compute_Z (this, xy);
      if (fabs (sag1 - sag2) > 1e-14)
	{
	  printf ("Error computing z - expected %.16f got %.16f\n", sag1, sag2);
	}
      return sag1;
    }

    void Asphere::derivative (const math::Vector2 &xy,
			      math::Vector2 &dxdy) const
    {
      Rotational::derivative (xy, dxdy);
      Vector2 dxdy2;
      compute_derivative (this, xy, dxdy2);
      if (fabs (dxdy.x () - dxdy2.x ()) > 1e-10
	  || fabs (dxdy.y () - dxdy2.y ()) > 1e-10)
	{
	  printf ("Derivative mismatch dx = %.16f %.16f, %.16f %.16f, dy %.16f "
		  "%.16f\n",
		  dxdy.x (), dxdy2.x (), dxdy.y (), dxdy2.y ());
	}
    }

    bool Asphere::intersect (math::Vector3 &point,
			     const math::VectorPair3 &ray) const
    {
      math::Vector3 result (0., 0, 0);
      math::Vector3 origin (ray.origin ().x (), ray.origin ().y (),
			    ray.origin ().z ());
      math::Vector3 direction (ray.direction ().x (), ray.direction ().y (),
			       ray.direction ().z ());
      compute_intersection (origin, direction, this, result);
      bool ok = Base::intersect (point, ray);
      if (fabs (point.x () - result.x ()) > 1e-10
	  || fabs (point.y () - result.y ()) > 1e-10
	  || fabs (point.z () - result.z ()) > 1e-10)
	{
	  printf ("%.16f %.16f, %.16f %.16f, %.16f %.16f\n", point.x (),
		  result.x (), point.y (), result.y (), point.z (),
		  result.z ());
	}
      point = result; // Let's use new method
      return ok;
    }

    void Asphere::normal (math::Vector3 &normal,
			  const math::Vector3 &point) const
    {
      Rotational::normal (normal, point);
      math::Vector3 N (0, 0, 0);
      compute_normal (this, point, N);
      N *= -1.0;
      if (fabs (normal.x () - N.x ()) > 1e-10
	  || fabs (normal.y () - N.y ()) > 1e-10
	  || fabs (normal.z () - N.z ()) > 1e-10)
	{
	  printf ("%.16f %.16f, %.16f %.16f, %.16f %.16f\n", normal.x (),
		  N.x (), normal.y (), N.y (), normal.z (), N.z ());
	}
      normal = N; // Lets use the new method
    }

  } // namespace curve
} // namespace goptical