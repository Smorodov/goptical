#define _NOMINMAX
#include <windows.h>

#ifndef GOPTICAL_RENDERER_OPENGL_HH_
#define GOPTICAL_RENDERER_OPENGL_HH_

extern "C"
{
#include <GL/gl.h>
}

#include "goptical/core/common.hpp"

#include "goptical/core/io/renderer_viewport.hpp"

namespace goptical
{

	namespace io
	{
		class Context
		{
			public:
			std::vector<float> line_vertices;
			std::vector< std::vector<GLuint> > line_indices;
			std::vector<float> line_colors;

			std::vector<float> triangle_vertices;
			std::vector<float> triangle_normals;
			std::vector<GLuint> triangle_indices;
			std::vector<float> triangle_colors;
			inline void clear()
			{
				    triangle_colors.clear();
					triangle_vertices.clear();
					triangle_normals.clear();
					triangle_indices.clear();

					line_vertices.clear();					
					for(auto& l : line_indices)
					{
						l.clear();
					}
					line_indices.clear();
					line_colors.clear();
			}
			Context(){;}
			~Context(){clear();}
		};
		/**
		   @short OpenGL rendering driver
		   @header <goptical/core/io/RendererOpengl
		   @module {Core}
		   @main

		   This class implements an Opengl graphic output driver. It needs
		   the opengl library to compile.
		 */
		class RendererOpengl : public RendererViewport
		{
			public:
				RendererOpengl (double Near, double Far, double width = 800,
				                double height = 600, const Rgb &background = rgb_black);

				
				RendererOpengl::~RendererOpengl();
				void set_z_range (double Near, double Far);

				static inline void glVertex (const math::Vector2 &v);
				static inline void glVertex (const math::Vector3 &v);
				static inline void glNormal (const math::Vector3 &v);
				static inline void glColor (const Rgb &rgb);
				static void apply_transform (const math::Transform<3> &t);
				static void get_transform (GLenum name, math::Transform<3> &t);

			private:
				/** @override */
				void clear ();
				/** @override */
				void flush ();

				/** @override */
				void set_2d_size (double width, double height);

				/** @override */
				void set_camera_transform (const math::Transform<3> &t);
				/** @override */
				math::Transform<3> get_camera_transform () const;
				/** @override */
				void set_orthographic ();
				/** @override */
				void set_perspective ();

				/** @override */
				void draw_point (const math::Vector2 &p, const Rgb &rgb, enum PointStyle s);
				/** @override */
				void draw_segment (const math::VectorPair2 &l, const Rgb &rgb);

				/** @override */
				void draw_point (const math::Vector3 &p, const Rgb &rgb, enum PointStyle s);
				/** @override */
				void draw_segment (const math::VectorPair3 &l, const Rgb &rgb);
				/** @override */
				void draw_polygon (const math::Vector3 *array, unsigned int count,
				                   const Rgb &rgb, bool filled, bool closed);
				/** @override */
				void draw_triangle (const math::Triangle<3> &t, const Rgb &rgb);
				/** @override */
				void draw_triangle (const math::Triangle<3> &t,
				                    const math::Triangle<3> &gradient, const Rgb &rgb);

				/** @override */
				void draw_text (const math::Vector3 &pos, const math::Vector3 &dir,
				                const std::string &str, TextAlignMask a, int size,
				                const Rgb &rgb);

				/** @override */
				void draw_text (const math::Vector2 &pos, const math::Vector2 &dir,
				                const std::string &str, TextAlignMask a, int size,
				                const Rgb &rgb);

				double _near, _far;
		};

		void
		RendererOpengl::glVertex (const math::Vector2 &v)
		{
			glVertex2d (v.x (), v.y ());
		}

		void
		RendererOpengl::glVertex (const math::Vector3 &v)
		{
			glVertex3d (v.x (), v.y (), v.z ());
		}

		void
		RendererOpengl::glNormal (const math::Vector3 &v)
		{
			glNormal3d (v.x (), v.y (), v.z ());
		}

		void
		RendererOpengl::glColor (const Rgb &rgb)
		{
			glColor4f (rgb.r, rgb.g, rgb.b, rgb.a);
		}
	}
}

#endif
