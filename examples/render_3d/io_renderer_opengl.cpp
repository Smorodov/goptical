
#include "renderer_opengl.hpp"
#include <goptical/core/math/transform.hpp>
#include <goptical/core/math/triangle.hpp>

extern "C"
{
#include <GL/glu.h>
}

namespace goptical
{

	namespace io
	{

		RendererOpengl::RendererOpengl (double Near, double Far, double width,  double height, const Rgb &bg)
			: _near (Near), _far (Far)
		{
			user_data=new Context();
			set_2d_size (width, height);
			_styles_color[StyleBackground] = bg;
			_styles_color[StyleForeground] = ~bg;
		}

		RendererOpengl::~RendererOpengl()
		{
			delete user_data;
		}

		void RendererOpengl::set_2d_size (double width, double height)
		{
			RendererViewport::set_2d_size (width, height);	
		}

		void RendererOpengl::set_camera_transform (const math::Transform<3> &t)
		{

		}

		math::Transform<3> RendererOpengl::get_camera_transform () const
		{
			return math::Transform<3>();
		}

		void RendererOpengl::set_orthographic ()
		{
			
		}

		void RendererOpengl::set_perspective ()
		{
			                
		}

		void RendererOpengl::set_z_range (double Near, double Far)
		{
			_near = Near;
			_far = Far;
		}

		void RendererOpengl::draw_point (const math::Vector2 &p, const Rgb &rgb,
		                            enum PointStyle s)
		{

		}

		void RendererOpengl::draw_segment (const math::VectorPair2 &l, const Rgb &rgb)
		{
					auto ctx=static_cast<Context*>(user_data);
			
			ctx->line_vertices.push_back(l[0].x());
			ctx->line_vertices.push_back(l[0].y());
			ctx->line_vertices.push_back(0);
			ctx->line_vertices.push_back(l[1].x());
			ctx->line_vertices.push_back(l[1].y());
			ctx->line_vertices.push_back(0);
			
			std::vector<GLuint> ind;
			if(ctx->line_indices.empty())
			{
			ind.push_back(0);
			ind.push_back(1);
			}else
			{
				GLuint last=(ctx->line_indices).back().back();
				ind.push_back(last+1);
				ind.push_back(last+2);
			}
			ctx->line_indices.push_back(ind);
		}

		void RendererOpengl::draw_point (const math::Vector3 &p, const Rgb &rgb,
		                            enum PointStyle s)
		{			
		}

		void RendererOpengl::draw_segment (const math::VectorPair3 &l, const Rgb &rgb)
		{
			auto ctx=static_cast<Context*>(user_data);
			
			ctx->line_vertices.push_back(l[0].x());
			ctx->line_vertices.push_back(l[0].y());
			ctx->line_vertices.push_back(l[0].z());
			ctx->line_vertices.push_back(l[1].x());
			ctx->line_vertices.push_back(l[1].y());
			ctx->line_vertices.push_back(l[1].z());
			
			std::vector<GLuint> ind;
			if(ctx->line_indices.empty())
			{
			ind.push_back(0);
			ind.push_back(1);
			}else
			{
				GLuint last=(ctx->line_indices).back().back();
				ind.push_back(last+1);
				ind.push_back(last+2);
			}
			ctx->line_indices.push_back(ind);
		}

		void RendererOpengl::draw_text (const math::Vector2 &c, const math::Vector2 &dir,
		                           const std::string &str, TextAlignMask a, int size,
		                           const Rgb &rgb)
		{
			auto ctx=static_cast<Context*>(user_data);
			// FIXME
		}

		void RendererOpengl::draw_text (const math::Vector3 &c, const math::Vector3 &dir,
		                           const std::string &str, TextAlignMask a, int size,
		                           const Rgb &rgb)
		{
			auto ctx=static_cast<Context*>(user_data);
			// FIXME
		}

		void RendererOpengl::draw_polygon (const math::Vector3 *array, unsigned int count, const Rgb &rgb, bool filled, bool closed)
		{
			auto ctx=static_cast<Context*>(user_data);
			if (count < 3)
			{
				return;
			}
			if (filled)
			{
				
			}
			else
			{

			}
		}

		void RendererOpengl::draw_triangle (const math::Triangle<3> &t, const Rgb &rgb)
		{
			auto ctx=static_cast<Context*>(user_data);
			auto N=t.normal();
			for (unsigned int i = 0; i < 3; i++)
			{				
				ctx->triangle_normals.push_back(N.x() );
				ctx->triangle_normals.push_back(N.y());
				ctx->triangle_normals.push_back(N.z());
				
				ctx->triangle_vertices.push_back(t[i].x() );
				ctx->triangle_vertices.push_back(t[i].y() );
				ctx->triangle_vertices.push_back(t[i].z() );
				
				ctx->triangle_indices.push_back( ctx->triangle_indices.size() );
			}
		}

		void RendererOpengl::draw_triangle (const math::Triangle<3> &t,
		                               const math::Triangle<3> &gradient,
		                               const Rgb &rgb)
		{
			auto ctx=static_cast<Context*>(user_data);
			for (unsigned int i = 0; i < 3; i++)
			{
				ctx->triangle_normals.push_back(gradient[i].x() );
				ctx->triangle_normals.push_back(gradient[i].y());
				ctx->triangle_normals.push_back(gradient[i].z());
				
				ctx->triangle_vertices.push_back(t[i].x() );
				ctx->triangle_vertices.push_back(t[i].y() );
				ctx->triangle_vertices.push_back(t[i].z() );
				
				ctx->triangle_indices.push_back( ctx->triangle_indices.size() );
			}
		}
		
		void RendererOpengl::clear ()
		{
		}

		void RendererOpengl::flush ()
		{			
		}
		
		void RendererOpengl::apply_transform (const math::Transform<3> &t)
		{		
		}

		void RendererOpengl::get_transform (GLenum name, math::Transform<3> &t)
		{			
		}

	}
}
