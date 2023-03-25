
#include <goptical/core/io/renderer_viewport.hpp>
#include <goptical/core/math/vector.hpp>

class App
{
	public:
		virtual ~App()
		{
		}

	protected:
		/** implemented by graphic library */
		virtual void main_loop() = 0;

		/** implemented by <goptical/core app */
		virtual void redraw() = 0;
		virtual void resize(int width, int height) = 0;

		goptical::math::Vector3 translation;
		goptical::math::Vector3 rotation;
		goptical::io::RendererViewport *renderer;
};

