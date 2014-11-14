#include <iostream>
#include "gl.h"
#include <memory>
#include "voxel_chunk.h"
#include "nd_array.h"
#include "shader_program.h"
#include "shader_data.h"
#include "uniform.h"
#include <initializer_list>
#include <glm/glm/gtc/quaternion.hpp>
#include <chrono>
#include <random>
#include "voxel_model.h"
#include "voxel_chunk_direct.h"
#include "timer.h"
#include "cut_voxel.h"
using std::unique_ptr;
using std::make_unique;
/*
create voxel_chunk holder

*/
struct Data
{
	std::unique_ptr<voxel_chunk_direct> voxel_direct;
	std::unique_ptr<voxel_chunk_direct> voxel_the_cutter;
	shader_program::ptr shader;

	glm::mat4 world;
	glm::mat4 transform;
	glm::mat4 base;
	GLfloat angle;
	const int size = 60;
	const float speed = 0.1f * size / 100.0f;
	const float rot_speed = 0.1f;
	int elapsed;
	glm::vec2 mouse;
	timer time;

	glm::mat4 s_transform;

	float dx;
	float dy;
	float dz;

	float drx;
	float dry;
};

std::unique_ptr<Data> data;

void keypress(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		data->dx = data->speed;
		break;
	case 'd':
		data->dx = -data->speed;
		break;
	case 'e':
		data->dy = -data->speed;
		break;
	case 'q':
		data->dy = data->speed;
		break;
	case 'w':
		data->dz = data->speed;
		break;
	case 's':
		data->dz = -data->speed;
		break;
	case 'j':
		data->dry = -data->rot_speed;
		break;
	case 'l':
		data->dry = data->rot_speed;
		break;
	case 'i':
		data->drx = -data->rot_speed;
		break;
	case 'k':
		data->drx = data->rot_speed;
		break;
	}
}
void key_up(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a':
		data->dx = 0.0;
		break;
	case 'd':
		data->dx = 0.0;
		break;
	case 'e':
		data->dy = 0.0;
		break;
	case 'q':
		data->dy = 0.0;
		break;
	case 'w':
		data->dz = 0.0;
		break;
	case 's':
		data->dz = 0.0;
		break;
	case 'j':
		data->dry = 0.0;
		break;
	case 'l':
		data->dry = 0.0;
		break;
	case 'i':
		data->drx = 0.0;
		break;
	case 'k':
		data->drx = 0.0;
		break;
	}
}

void idle()
{
	auto change = glutGet(GLUT_ELAPSED_TIME) - data->elapsed;
	data->elapsed += change;
	data->angle += 0.0005f * change;
	auto new_pos = glm::vec3{ sin(data->angle) * data->size * 2 - data->size / 2, cos(data->angle) * data->size * 2 + data->size, 0 };
	data->s_transform = glm::translate(glm::vec3(1.0f / 3.0f) * new_pos);
	glm::vec2 middle = glm::vec2{ 400, 400 };


	data->transform = glm::rotate(change * data->drx * 0.01f, glm::vec3{ 1, 0, 0 }) * data->transform;
	data->transform = glm::rotate(change * data->dry * 0.01f, glm::vec3{ 0, 1, 0 })* data->transform;
	data->transform = glm::translate(glm::vec3{ data->dx, data->dy, data->dz } *(float)change) * data->transform;

	voxel::difference(glm::vec3(), *data->voxel_direct, new_pos, *data->voxel_the_cutter);

	data->voxel_direct->update();
	data->voxel_the_cutter->update();
	glutPostRedisplay();
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	data->voxel_direct->draw(data->base, data->transform);

	data->voxel_the_cutter->draw(data->base, data->transform, data->s_transform);
	glutSwapBuffers();
}
void mouse(int x, int y)
{
	data->mouse = glm::vec2{ x, y };
}

int main(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_STENCIL | GLUT_DOUBLE);
	glutInitWindowSize(800, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glEnable(GLUT_MULTISAMPLE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glutCreateWindow("Wahahahaha");

	std::cout << glGetString(GL_VENDOR) << std::endl <<
		glGetString(GL_RENDERER) << std::endl <<
		glGetString(GL_VERSION) << std::endl <<
		glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// required to have for some of the extensions used here work
	glewExperimental = true;

	if (glewInit() != GLEW_OK)
	{
		return 1;
	}
	glClearColor(0.3f, 0.2f, 0.3f, 1.0f);

	auto clock = std::chrono::steady_clock();

	std::default_random_engine def;

	data = make_unique<Data>();

	auto s = std::chrono::time_point_cast<std::chrono::nanoseconds>(clock.now());
	def.seed(static_cast<unsigned long>(s.time_since_epoch().count()));
	std::uniform_int<> rand(0, data->size / 5);

	data->angle = 0.0f;	

	data->voxel_direct = std::make_unique<voxel_chunk_direct>(data->size, data->size, data->size);
	data->voxel_the_cutter = std::make_unique<voxel_chunk_direct>(data->size, data->size, data->size);

	for (int x = 0; x < data->size; x++)
	{
		for (int y = 0; y < data->size; y++)
		{
			for (int z = 0; z < data->size * 5; z++)
			{
				auto recentered_x = -x + data->size / 2;
				auto recentered_y = -y + data->size / 2;
				auto recentered_z = -z + data->size / 2;
				if (sqrt(recentered_x * recentered_x + recentered_y* recentered_y) <= data->size / 2
					&& ((z * x) / (y ? y : 1)) <= data->size / 2
					)
				{
					data->voxel_direct->on(x, y, z);
					data->voxel_the_cutter->on(x, y, z);
				}

			}
		}
	}

	data->time.stop();
	data->time.start();
	auto time = data->time.lap();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(time).count() << std::endl;
	shader::ptr vertex = make_shader(shaders::voxel::vertex, GL_VERTEX_SHADER);
	shader::ptr geometry = make_shader(shaders::voxel::geometry, GL_GEOMETRY_SHADER);
	shader::ptr fragment = make_shader(shaders::voxel::fragment, GL_FRAGMENT_SHADER);

	float zNear = .01f;
	float zFar = 600.0f;

	data->shader = make_program({ vertex, geometry, fragment });
	glUseProgram(voxel_chunk::voxel_shader().id);
	data->base =
		glm::perspective(glm::radians(60.0f), 1.0f, zNear, zFar)
		;

	data->dx = 0;
	data->dy = 0;
	data->dz = 0;
	data->drx = 0;
	data->dry = 0;

	std::vector<glm::vec3> lights;


	for (int i = -1; i < 2; i += 2)
	{
		for (int j = -1; j < 2; j += 2)
		{
			for (int k = -1; k < 2; k += 2)
			{
				lights.push_back(glm::vec3{
					i * rand(def) / 2.0f
					, j * rand(def) / 2.0f
					, k * rand(def) / 2.0f
					
				});
			}
		}
	}
	lights.push_back(glm::vec3(0, 0, data->size + 2));

	glUseProgram(voxel_chunk_direct::shader().id);

	uniform(voxel_chunk_direct::shader(), "lights", lights);
	uniform(voxel_chunk_direct::shader(), "num_lights", lights.size());



	data->transform = glm::translate(glm::vec3(0, 0, -data->size - 2));

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDepthRange(0.0f, 1.0f);
	glClearDepth(1.0f);

	glutIdleFunc(&idle);
	glutDisplayFunc(&draw);

	glutKeyboardFunc(&keypress);
	glutKeyboardUpFunc(&key_up);
	glutPassiveMotionFunc(mouse);

	data->elapsed = glutGet(GLUT_ELAPSED_TIME);

	glutMainLoop();

	return 0;
}