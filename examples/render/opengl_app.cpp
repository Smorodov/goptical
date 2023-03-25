#include <math.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <goptical/core/io/renderer_opengl.hpp>

#include "app.hpp"
#include "newton.hpp"

using namespace goptical;

class OpenglApp : public App
{
	public:
		OpenglApp();
		~OpenglApp();

	protected:
		void main_loop();

	private:
		static void glut_keys(unsigned char key, int x, int y);
		static void glut_mouse_func(int button, int state, int x, int y);
		static void glut_mouse_motion(int x, int y);
		static void gl_draw();
		static void gl_resize(GLsizei width, GLsizei height);

		static OpenglApp * app;
		int mouse_x, mouse_y;
		int mouse_bt;
};

OpenglApp * OpenglApp::app;

OpenglApp::OpenglApp()
{
	char *argv = (char*)"a.out";
	int argc = 1;
	mouse_bt = 0;
	app = this;
	glutInit( &argc, &argv );
	glutInitWindowSize( 640, 480 );
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("goptical/core OpenGL renderer");
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	renderer = new io::RendererOpengl(20., 100000., 640, 480);
	glutReshapeFunc(gl_resize);
	glutKeyboardFunc(glut_keys);
	glutPassiveMotionFunc(glut_mouse_motion);
	glutMotionFunc(glut_mouse_motion);
	glutMouseFunc(glut_mouse_func);
	glutDisplayFunc(gl_draw);
}

OpenglApp::~OpenglApp()
{
	delete renderer;
}

void OpenglApp::main_loop()
{
	glutMainLoop();
}

void OpenglApp::glut_keys(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
		case 27:
			exit(0);
		case 'w':                   // wireframe
			glEnable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_BLEND);
			glutPostRedisplay();
			break;
		case 'f':                   // filled
			glEnable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_BLEND);
			glutPostRedisplay();
			break;
		case 'b':
			glDisable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_BLEND);
			glutPostRedisplay();
			break;
	}
}

void OpenglApp::glut_mouse_func(int button, int state, int x, int y)
{
	int s;
	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			s = 1;
			break;
		case GLUT_MIDDLE_BUTTON:
			s = 2;
			break;
		case GLUT_RIGHT_BUTTON:
			s = 4;
			break;
		default:
			return;
	}
	if (state == GLUT_UP)
	{
		app->mouse_bt &= ~s;
	}
	else
	{
		app->mouse_bt |= s;
	}
}

void OpenglApp::glut_mouse_motion(int x, int y)
{
	GLfloat dx = GLfloat(app->mouse_x - x);
	GLfloat dy = GLfloat(app->mouse_y - y);
	if (app->mouse_bt & 1)
	{
		app->translation.x() -= dx;
		app->translation.y() += dy;
	}
	else if (app->mouse_bt & 2)
	{
		app->translation.z() += dx;
	}
	else if (app->mouse_bt & 4)
	{
		app->rotation.y() += 0.1 * dx;
		app->rotation.x() += 0.1 * dy;
	}
	if (app->mouse_bt)
	{
		glutPostRedisplay();
	}
	app->mouse_x = x;
	app->mouse_y = y;
}

void OpenglApp::gl_draw()
{
	app->redraw();
	glutSwapBuffers();
}

void OpenglApp::gl_resize(GLsizei width, GLsizei height)
{
	if (height == 0)
	{
		height++;
	}
	app->resize(width, height);
}

int main(int argc, char **argv)
{
	Newton<OpenglApp> app;
	return 0;
}


