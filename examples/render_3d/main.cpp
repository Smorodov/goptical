// http://rodolphe-vaillant.fr/?e=29

#define GL_GLEXT_PROTOTYPES
#include <windows.h>		

#include <time.h>
#include "stdlib.h"
#include <crtdbg.h>
#include <conio.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <cmath>
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <set>
#include <vector>


#include <goptical/core/sys/image.hpp>
#include <goptical/core/sys/source_point.hpp>
#include <goptical/core/sys/source_disk.hpp>
#include <goptical/core/sys/surface.hpp>
#include <goptical/core/sys/system.hpp>

#include <goptical/core/trace/distribution.hpp>
#include <goptical/core/trace/params.hpp>
#include <goptical/core/trace/result.hpp>
#include <goptical/core/trace/sequence.hpp>
#include <goptical/core/trace/tracer.hpp>

#include <goptical/core/math/quaternion.hpp>
#include <goptical/core/math/transform.hpp>
#include <goptical/core/math/vector.hpp>
#include "goptical/core/material/air.hpp"

#include <goptical/core/analysis/rayfan.hpp>
#include <goptical/core/analysis/spot.hpp>
#include <goptical/core/curve/sphere.hpp>
#include <goptical/core/data/plot.hpp>
#include <goptical/core/io/renderer_svg.hpp>
#include "renderer_opengl.hpp"
#include <goptical/core/io/rgb.hpp>
#include <goptical/core/light/spectral_line.hpp>
#include <goptical/core/material/abbe.hpp>
#include <goptical/core/material/sellmeier.hpp>
#include <goptical/core/material/base.hpp>

#include <goptical/core/shape/disk.hpp>
#include <goptical/core/sys/image.hpp>
#include <goptical/core/sys/lens.hpp>
#include <goptical/core/sys/source.hpp>
#include <goptical/core/sys/source_point.hpp>
#include <goptical/core/sys/source_rays.hpp>
#include <goptical/core/sys/stop.hpp>
#include <goptical/core/trace/distribution.hpp>
#include <goptical/core/trace/params.hpp>
#include <goptical/core/trace/result.hpp>
#include <goptical/core/trace/sequence.hpp>
#include <goptical/core/trace/tracer.hpp>

using namespace goptical;

std::shared_ptr<sys::System> opt_sys;
std::shared_ptr<trace::Tracer> tracer;
io::Renderer* renderer;

int tex_size = 1024;

using namespace std;


// Create shader
char const* vsrc = "\
    #version 330 core                                                           \n\
    layout(location = 0) in vec3 position;                                      \n\
    layout(location = 1) in vec3 normal;                                        \n\
    out vec3 data_normal;                                                               \n\
    out vec3 data_position;                                                               \n\
    out mat4 data_mvp;\n\
    uniform mat4 mvp;                                              \n\
    void main() {                                                               \n\
      data_position=vec3(mvp * vec4(position, 1.0));                                      \n\
      gl_Position=vec4(data_position,1.0);                                                \n\
      data_mvp=mvp;\n\
      data_normal = (vec4(normal, 1.0)).xyz;                              \n\
    }                                                                           \n\
  ";

char const* fsrc = "\
    #version 330                                                              \n\
    in vec3 data_normal; \n\
    in vec3 data_position; \n\
    in mat4 data_mvp; \n\
    out vec4 outputF;                                                             \n\
    void main()                                                                 \n\
    { \n\
    vec3 rgb = vec3(0.8,0.8,0.8);                                               \n\
    float ambient = 0.2;\n\
    mat4 mvp=transpose(data_mvp); \n\
    vec3 L=normalize(vec3(mvp*vec4(0,0,-1,0)));\n\
    vec3 eye=normalize(vec3(0,0,-1));\n\                                                \n\
    vec3 E = normalize(eye - data_position);\n\
    vec3 H = normalize(L + E);\n\
    vec3 N = data_normal;\n\
	float diffuse = abs(dot(N, L));\n\
    float specular = pow(max(abs(dot(N, H)), 0.0), 12.0);\n\
    float NE = max(abs(dot(N, E)), 0.0);\n\
    float fresnel = pow(sqrt(1.0 - NE * NE), 12.0);\n\
    outputF = vec4((0.5 * diffuse + 0.4 * specular + 0.6 * fresnel + ambient) * rgb, 1.0);\n\
    }\n\
  ";



class MyTrackBall
{
public:
	// Âûñîòà è øèðèíà îêíà îòîáðàæåíèÿ
	int width;
	int height;
	int	gMouseX;
	int	gMouseY;
	glm::vec2 lastPos;
	double	Vvx, Vvy, Vvz;
	double	Vglx, Vgly, Vglz;
	double	Vprx, Vpry, Vprz;
	double	mx0, my0, mx1, my1;
	double  vx1, vy1, vz1;
	double	ViewMatrix[16];
	double	xp, yp, zp;
	double	dx, dy, dz;
	double	rx, ry, rz;
	double	kx;
	int		button;
	double	obj_size;
	double	X0, Y0, Z0;
	double	Pi;
	double	deg;
	double	last_ang;
	double	X, Y, Z;
	double	xRot;
	double	yRot;
	double	zRot;

	MyTrackBall(int w, int h, float obj_size = 100)
	{
		width = w;
		height = h;
		gMouseX = 0;
		gMouseY = 0;
		button = 0;
		lastPos = glm::vec2(0.0f, 0.0f);
		this->obj_size = obj_size;
		X0 = 0, Y0 = 0, Z0 = 0;
		Pi = 3.14159265358979323846;
		deg = Pi / 180;
		last_ang = 0;
		X = 0, Y = 0, Z = 0;

		xRot = 0;
		yRot = 0;
		zRot = 0;
		dx = 0; dy = 0; dz = 0;
		xp = 0; yp = 0; zp = 0;
		rx = 0; ry = 0; rz = 0;
		mx0 = 0; my0 = 0;
		mx1 = 0; my1 = 0;
		button = 0;
		Vvx = 0; Vvy = 1; Vvz = 0;
		Vglx = 0; Vgly = 0; Vglz = 1;
		Vprx = 1; Vpry = 0; Vprz = 0;
		if (width < height) { kx = 0.5 * obj_size / height; }
		else { kx = 0.5 * obj_size / width; }
	}

	//--------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------
	void MouseWheelCallback(int direction)
	{
		if (direction > 0)
		{
			obj_size *= 0.95;
		}
		else if (direction < 0)
		{
			obj_size *= 1.05;
		}
	}
	//--------------------------------------------------------------------------  
	// Ôóíêöèÿ îáðàáîòêè íàæàòèé êíîïîê ìûøè
	// mouse left = 1
	// mouse right = 2
	// mouse middle = 3
	//--------------------------------------------------------------------------
	void MouseButtonCallback1(int button, int x, int y)
	{
		this->button = button;
		dx = 0;
		dy = 0;
		lastPos.x = x;
		lastPos.y = y;
	}

	//--------------------------------------------------------------------------
	// Ôóíêöèÿ îáðàáîòêè ïåðåìåùåíèé ìûøè
	//-------------------------------------------------------------------------- 
	void MouseMotionCallback(int x, int y)
	{
		if (button > 0)
		{
			dx = x - lastPos.x;
			dy = y - lastPos.y;
			lastPos.x = x;
			lastPos.y = y;
		}
		else
		{
			dx = 0;
			dy = 0;
		}
	}

	glm::mat4 getMVP(void)
	{
		double nRange = obj_size;
		int w = width;
		int h = height;
		if (h == 0) { h = 1; }
		glm::vec4 viewport = glm::vec4(0, 0, w, h);
		glm::mat4 projection;
		if (w <= h)
		{
			projection = glm::ortho(-nRange, nRange, -nRange * h / w, nRange * h / w, -200 * nRange, 200 * nRange);
		}
		else
		{
			projection = glm::ortho(-nRange * w / h, nRange * w / h, -nRange, nRange, -200 * nRange, 200 * nRange);
		}
		glm::mat4 view(1.0f); // Identity matrix         
		double wx1, wy1, wz1;
		glm::vec3 projected = glm::project(glm::vec3(xp, yp, zp), view, projection, viewport);
		vx1 = projected.x;
		vy1 = projected.y;
		vz1 = projected.z;
		//gluProject(xp, yp, zp, &modelview1[0], &projection1[0], &viewport1[0], &vx1, &vy1, &vz1);

		if (button == 3)
		{
			vx1 = vx1 + dx;
			vy1 = vy1 - dy;
		}
		glm::vec3 unprojected = glm::unProject(glm::vec3(vx1, vy1, 0), view, projection, viewport);
		wx1 = unprojected.x;
		wy1 = unprojected.y;
		wz1 = unprojected.z;
		//gluUnProject(vx1, vy1, 0, modelview1, projection1, viewport1, &wx1, &wy1, &wz1);
		if (button == 3 && wx1 != 0 && wy1 != 0)
		{
			xp = wx1;
			yp = wy1;
			zp = 0;
		}
		view = glm::translate(view, glm::vec3(xp, yp, zp));

		if (button == 1)
		{
			double LVgl = 1, LVv = 1, LVpr = 1;
			dx /= 100;
			Vprx = Vprx - Vglx * (-dx);
			Vpry = Vpry - Vgly * (-dx);
			Vprz = Vprz - Vglz * (-dx);
			dy /= 100;
			Vvx = Vvx - Vglx * (dy);
			Vvy = Vvy - Vgly * (dy);
			Vvz = Vvz - Vglz * (dy);

			Vglx = Vpry * Vvz - Vprz * Vvy;
			Vgly = Vprz * Vvx - Vprx * Vvz;
			Vglz = Vprx * Vvy - Vpry * Vvx;

			Vprx = Vvy * Vglz - Vvz * Vgly;
			Vpry = Vvz * Vglx - Vvx * Vglz;
			Vprz = Vvx * Vgly - Vvy * Vglx;

			LVgl = sqrt(Vglx * Vglx + Vgly * Vgly + Vglz * Vglz);
			LVv = sqrt(Vvx * Vvx + Vvy * Vvy + Vvz * Vvz);
			LVpr = sqrt(Vprx * Vprx + Vpry * Vpry + Vprz * Vprz);

			if (LVgl != 0)
			{
				Vglx = Vglx / LVgl;
				Vgly = Vgly / LVgl;
				Vglz = Vglz / LVgl;
			}
			if (LVpr != 0)
			{
				Vprx = Vprx / LVpr;
				Vpry = Vpry / LVpr;
				Vprz = Vprz / LVpr;
			}
			if (LVv != 0)
			{
				Vvx = Vvx / LVv;
				Vvy = Vvy / LVv;
				Vvz = Vvz / LVv;
			}
		}
		glm::mat4 view_rot;
		if (glm::vec3(Vglx, Vgly, Vglz).length() > 0)
		{
			view_rot = glm::lookAtRH(glm::vec3(Vglx, Vgly, Vglz), glm::vec3(0, 0, 0), glm::vec3(Vvx, Vvy, Vvz));
		}
		else
		{
			view_rot = glm::mat4(1.0f);
		}
		view = view * view_rot;
		// view = glm::translate(view, glm::vec3(X0, Y0, Z0));
		 //-------------------------------------------------------------------------       
		dx = 0;
		dy = 0;
		glm::mat4 model(1.0f);
		glm::mat4 MVP = projection * view * model;
		return MVP;
	}

};
MyTrackBall* trackball;

void KeyboardCallback(unsigned char key, int x, int y);
void initialize(void);
void finalize(void);


// Vertex buffer
GLuint vertices = 0;
// Vertex normals
GLuint normals = 0;
// Vertex indices
GLuint indices = 0;
GLuint program;
GLuint vs;
GLuint fs;


//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------

void RenderString(float xp, float yp, float zp, void* font, const char* string, float r, float g, float b)
{
	glColor3f(r, g, b);
	glRasterPos3f(xp, yp, zp);
	glutBitmapString(font, (unsigned char*)string);
}
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------

void RenderTransform(float* t)

{

	glLineWidth(3);
	float scale = 0.01;
	float ox = t[4 * 0 + 3];
	float oy = t[4 * 1 + 3];
	float oz = t[4 * 2 + 3];
	float xx = (t[4 * 0 + 0] - ox) * scale;
	float xy = (t[4 * 1 + 0] - oy) * scale;
	float xz = (t[4 * 2 + 0] - oz) * scale;
	float yx = (t[4 * 0 + 1] - ox) * scale;
	float yy = (t[4 * 1 + 1] - oy) * scale;
	float yz = (t[4 * 2 + 1] - oz) * scale;
	float zx = (t[4 * 0 + 2] - ox) * scale;
	float zy = (t[4 * 1 + 2] - oy) * scale;
	float zz = (t[4 * 2 + 2] - oz) * scale;

	xx += ox;
	xy += oy;
	xz += oz;
	yx += ox;
	yy += oy;
	yz += oz;
	zx += ox;
	zy += oy;
	zz += oz;
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(ox, oy, oz);
	glVertex3f(xx, xy, xz);
	glColor3f(0, 1, 0);
	glVertex3f(ox, oy, oz);
	glVertex3f(yx, yy, yz);
	glColor3f(0, 0, 1);
	glVertex3f(ox, oy, oz);
	glVertex3f(zx, zy, zz);
	glEnd();
	glLineWidth(1);
}

//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
float* vetexNormals = 0;
float* vertices_data = 0;
unsigned int* faceIndices = 0;
GLuint vertex_num = 0;
GLint face_index_num = 0;

void assignContext(io::Context* ctx)
{
	vertex_num = ctx->triangle_vertices.size() / 3;
	face_index_num = ctx->triangle_indices.size() / 3;
	vetexNormals = (ctx->triangle_normals.data());
	vertices_data = (ctx->triangle_vertices.data());
	faceIndices = (ctx->triangle_indices.data());


}


//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------

void display(void)
{
	glOrtho(-1, 1, -1, 1, -1, 1);
	/*
	for (int i = 0; i <  skeleton.joint_abs_transforms.shape[0]; ++i)
	{
		float* t= joint_rel_transforms +16*i;
		float ox = t[4 * 0 + 3];
		float oy = t[4 * 1 + 3];
		float oz = t[4 * 2 + 3];
		RenderString(ox, oy, oz, GLUT_BITMAP_TIMES_ROMAN_24, std::to_string(i).c_str(), 1.0f, 1.0f, 0.0f);
		RenderTransform(t);
	}

	glColor3f(1, 1, 0);
	glLineWidth(3);
	glBegin(GL_LINES);
	for (int i = 0; i < skeleton_indices.size(); ++i)
	{
		int i1 = skeleton_indices[i];
		float* t = joint_rel_transforms + 16 * i1;
		float ox = t[4 * 0 + 3];
		float oy = t[4 * 1 + 3];
		float oz = t[4 * 2 + 3];
		glVertex3f(ox,oy,oz);
	}
	glEnd();
	glLineWidth(1);
	*/

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	// Draw mesh

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, -1);
	glOrtho(1, -1, 1, -1, -1, 1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_num * 3 * sizeof(float), vertices_data);
	glBindBuffer(GL_ARRAY_BUFFER, normals);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_num * 3 * sizeof(float), vetexNormals);
	glBindBuffer(GL_ARRAY_BUFFER, indices);
	glBufferSubData(GL_ARRAY_BUFFER, 0, face_index_num * 3 * sizeof(int), (unsigned int*)faceIndices);
	glUseProgram(program);
	glm::mat4 mvp = trackball->getMVP();
	glUniformMatrix4fv(glGetUniformLocation(program, "mvp"), 1, false, glm::value_ptr(mvp));
	glDrawElements(GL_TRIANGLES, face_index_num * 3, GL_UNSIGNED_INT, 0);
	glUseProgram(0);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	//glOrtho(-1, 1, -1, 1, -1, 1);
	glDisable(GL_LIGHTING);


	glColor3f(0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, face_index_num * 3, GL_UNSIGNED_INT, 0);


	glColor3f(0, 0, 1);


	//  glPointSize(6);
	//  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	//  glDrawElements(GL_POINTS,  face_index_num*3, GL_UNSIGNED_INT, 0);
	//  glPointSize(1);



 // normals
	 /*
	 glBegin(GL_POINTS);
	 for (int i = 0; i <  vertex_num; ++i)
	 {
		 float x = vertices_data[3 * i + 0];
		 float y = vertices_data[3 * i + 1];
		 float z = vertices_data[3 * i + 2];
		 glVertex3f(x, y, z);
	 }
	 glEnd();
	 glPointSize(1);

	 glColor3f(1, 1, 0);
	 glBegin(GL_LINES);
	 for (int i = 0; i <  vertex_num; ++i)
	 {
		 float x = vertices_data[3 * i + 0];
		 float y = vertices_data[3 * i + 1];
		 float z = vertices_data[3 * i + 2];
		 float nx = vetexNormals[i * 3 + 0]/300;
		 float ny = vetexNormals[i * 3 + 1]/300;
		 float nz = vetexNormals[i * 3 + 2]/300;
		 glVertex3f(x, y, z);
		 glVertex3f(x+nx, y+ny, z+nz);
	 }
	 glEnd();
	 */

	 /*
	 for (int i = 0; i <  vertex_num; ++i)
	 {
		 float x =vertices_data[3 * i + 0];
		 float y =vertices_data[3 * i + 1];
		 float z =vertices_data[3 * i + 2];
		 RenderString(x, y, z, GLUT_BITMAP_TIMES_ROMAN_10, std::to_string(i).c_str(), 1.0f, 1.0f, 0.0f);
	 }
	 */
	auto ctx = static_cast<io::Context*>(renderer->user_data);
	glColor3f(1, 1, 1);
	for (auto l : ctx->line_indices)
	{
		glBegin(GL_LINES);
		for (auto p : l)
		{
			auto vx = ctx->line_vertices[p * 3 + 0];
			auto vy = ctx->line_vertices[p * 3 + 1];
			auto vz = ctx->line_vertices[p * 3 + 2];
			glVertex3f(vx, vy, vz);
		}
		glEnd();
	}



}


//--------------------------------------------------------------------------
// KEYS
//--------------------------------------------------------------------------
static void KeyboardCallback(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case 'w':
	case 'W':
		break;
	}
}



//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
void Arrow(float x1, float y1, float z1, float x2, float y2, float z2)
{
	float l = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
	glPushMatrix();
	glTranslatef(x1, y1, z1);
	if (l != 0) { glRotatef(180, (x2 - x1) / (2 * l), (y2 - y1) / (2 * l), (z2 - z1 + l) / (2 * l)); }
	GLUquadricObj* quadObj;
	quadObj = gluNewQuadric();
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluCylinder(quadObj, l / 20, l / 20, l, 8, 1);
	glTranslatef(0, 0, l);
	gluCylinder(quadObj, l / 10, 0, l / 4, 8, 1);
	glPopMatrix();
	gluDeleteQuadric(quadObj);
}
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
void MouseWheelCallback(int wheel, int direction, int x, int y)
{
	trackball->MouseWheelCallback(direction);
}
//--------------------------------------------------------------------------  
// Ôóíêöèÿ îáðàáîòêè íàæàòèé êíîïîê ìûøè
//--------------------------------------------------------------------------
static void MouseCallback1(int _button, int state, int x, int y)
{
	int button = 0;
	if (_button == GLUT_LEFT_BUTTON) { button = 1; }
	if (_button == GLUT_MIDDLE_BUTTON) { button = 3; }
	if (_button == GLUT_RIGHT_BUTTON) { button = 2; }
	trackball->MouseButtonCallback1(button, x, y);
}

//--------------------------------------------------------------------------
// Ôóíêöèÿ îáðàáîòêè ïåðåìåùåíèé ìûøè
//-------------------------------------------------------------------------- 
static void MotionCallback(int x, int y)
{
	trackball->MouseMotionCallback(x, y);
}
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
static void ArrowKeyCallback(int key, int x, int y)
{
	KeyboardCallback(key, x, y);
}
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
static void IdleCallback()
{
	glutPostRedisplay();
}
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
void renderScene(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glm::mat4 mvp = trackball->getMVP();
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(mvp));
	// Render all here    
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	display();
	glPopMatrix();
	//--------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------
	glFlush();
	glutSwapBuffers();
}


void buildSystem(std::shared_ptr<sys::System>& opt_sys)
{

	// light source
	auto source = std::make_shared<sys::SourcePoint>(sys::SourceAtInfinity, math::Vector3(0, 0, 1));
	auto image = std::make_shared<sys::Image>(math::Vector3(0, 0, 140.35),30);

	auto lens1 = std::make_shared<sys::Lens>(math::Vector3(0, 0, 0));
	lens1->add_surface(30.0, 15.0, 4, std::make_shared<material::AbbeVd>(1.607170, 59.5002));
	lens1->add_surface(60.0, 15.0, 4);

	auto lens2 = std::make_shared<sys::Lens>(math::Vector3(0, 0, 50));
	lens2->add_surface(-50.0, 15.0, 4, std::make_shared<material::AbbeVd>(1.607170, 59.5002));
	lens2->add_surface(60.0, 15.0, 4);

	opt_sys->add(source);
	opt_sys->add (lens1);	
	opt_sys->add (lens2);	
	opt_sys->add(image);
	
	auto seq = std::make_shared<trace::Sequence>(*opt_sys);
	opt_sys->get_tracer_params().set_sequential_mode(seq);
	std::cout << "opt_system:" << std::endl;
	std::cout << opt_sys;
	std::cout << "sequence:" << std::endl << seq;
	
	// trace and draw rays from rays source
	opt_sys->enable_single<sys::Source>(*source);
	tracer = std::make_shared<trace::Tracer>(opt_sys.get());
	tracer->get_trace_result().set_generated_save_state(*source);
	tracer->trace();
}

void update()
{
	// fill arrays     
	auto ctx = static_cast<io::Context*>(renderer->user_data);
	ctx->clear();
	opt_sys->draw_3d(*renderer);
	// draw rays
	auto& res = tracer->get_trace_result();
	res.draw_3d(*renderer);
	assignContext(ctx);
	// ser GPU buffers
	glDeleteBuffers(1, &vertices);
	glDeleteBuffers(1, &normals);
	glDeleteBuffers(1, &indices);

	glGenBuffers(1, &vertices);
	glGenBuffers(1, &normals);
	glGenBuffers(1, &indices);

	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * 3 * sizeof(float), NULL, GL_STREAM_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, normals);
	glBufferData(GL_ARRAY_BUFFER, vertex_num * 3 * sizeof(float), NULL, GL_STREAM_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_index_num * 3 * sizeof(int), NULL, GL_STREAM_DRAW);


}


//--------------------------------------------------------------------------
// MAIN
//--------------------------------------------------------------------------
void main(int argc, char** argv)
{
	system("chcp 1251");
	system("cls");
	int width = 800, height = 800;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	trackball = new MyTrackBall(width, height, 10);
	// Create window, set callbacks
	int mainHandle = glutCreateWindow("3D View");
	glutSetWindow(mainHandle);
	glutDisplayFunc(renderScene);
	glutIdleFunc(IdleCallback);
	glutKeyboardFunc(KeyboardCallback);
	glutSpecialFunc(ArrowKeyCallback);
	glutMouseFunc(MouseCallback1);
	glutMotionFunc(MotionCallback);
	glutMouseWheelFunc(MouseWheelCallback);
	glutCloseFunc(finalize);
	MotionCallback(0, 0);

	// Setup default render states
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	float ambientColor[] = { 0.5f, 0.5f, 0.5f, 0.0f };
	float diffuseColor[] = { 1.0f, 1.0f, 1.0f, 0.0f };
	float specularColor[] = { 0.5f, 0.5f, 0.5f, 0.0f };
	float position[] = { -1000.0f, 100.0f, 400.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHT0);

	// -----------------
	opt_sys = std::make_shared<sys::System>();
	buildSystem(opt_sys);
	auto bb = opt_sys->get_bounding_box();

	renderer = (io::Renderer*) new io::RendererOpengl(20., 100000., width, height);

	//------------------
	initialize();
	update();

	glutMainLoop();
}



//--------------------------------------------------------------------------
// START
//--------------------------------------------------------------------------
void initialize(void)
{
	glewInit();

	program = glCreateProgram();
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vsrc, NULL);
	glCompileShader(vs);
	glAttachShader(program, vs);
	glDeleteShader(vs);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fsrc, NULL);
	glCompileShader(fs);
	glAttachShader(program, fs);
	glDeleteShader(fs);
	glLinkProgram(program);


}

//--------------------------------------------------------------------------
// END
//--------------------------------------------------------------------------
void finalize(void)
{
	glDeleteProgram(program);
	glDeleteBuffers(1, &vertices);
	glDeleteBuffers(1, &normals);
	glDeleteBuffers(1, &indices);
	delete trackball;

}





