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
#include "timer.h"
using std::unique_ptr;
using std::make_unique;
/*
Create subdivide,
	-a subdivide size
	-a render size

	-figure out how to center properly

scale the models properly,
create voxel_chunk holder

*/
struct Data
{
	std::unique_ptr<voxel_chunk> voxel;
	shader_program::ptr shader;

	glm::mat4 world;
	glm::mat4 transform;
	glm::mat4 base;
	GLfloat angle;
	const int size = 150;
	int elapsed;
	glm::vec2 mouse;
	timer time;
};

std::unique_ptr<Data> data;

void idle()
{
	auto change = glutGet(GLUT_ELAPSED_TIME) - data->elapsed;
	data->elapsed += change;
	data->angle += 0.0005f * change;

	glm::vec2 middle = glm::vec2{ 400, 400 };
	auto diff = data->mouse - middle;
	auto invert = glm::vec2{ -diff.y, diff.x };
	//data->rotate *= glm::rotate(glm::radians( change * .1f), glm::vec3(invert, 0));

	data->transform =
		
		 glm::translate(glm::vec3(0, 0, -data->size - 2))
	//	 * glm::rotate(-data->angle, glm::vec3{ 1.0f, 0.0f, 0.0f })
		;

	data->voxel->transform = glm::rotate(data->angle, glm::vec3{ 1.0f, 0.0f, 0.0f });

	data->voxel->set(!data->voxel->get(0, 0, 0), 0, 0, 0);
	data->time.start();
	data->voxel->update();
	data->time.stop();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(data->time.duration()).count() << std::endl;
	glutPostRedisplay();
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	data->voxel->draw(data->base, data->transform);
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
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

	data = make_unique<Data>();
	data->voxel = make_unique<voxel_chunk>(data->size, data->size, data->size);
	data->angle = 0.0f;
	auto& voxel_data = *data->voxel;
	for (int x = 0; x < data->size; x++)
	{
		for (int y = 0; y < data->size; y++)
		{
			for (int z = 0; z < data->size; z++)
			{
				auto recentered_x = -x + data->size / 2;
				auto recentered_y = -y + data->size / 2;
				auto recentered_z = -z + data->size / 2;
				if (sqrt(recentered_x * recentered_x + recentered_y* recentered_y + recentered_z * recentered_z) <= data->size / 2
					&& (z * x / (y ? y : 1)) <= data->size / 2
					) // abs(x + y - z) <= 5
				
					voxel_data.on(x, y, z);
			}
		}
	}

	auto clock = std::chrono::steady_clock();

	voxel_data.get(0, 0, 0);
	auto begin = clock.now();
	voxel_data.update();
	auto end = clock.now();
	auto diff = end - begin;
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << std::endl;
	shader::ptr vertex = make_shader(shaders::voxel::vertex, GL_VERTEX_SHADER);
	shader::ptr geometry = make_shader(shaders::voxel::geometry, GL_GEOMETRY_SHADER);
	shader::ptr fragment = make_shader(shaders::voxel::fragment, GL_FRAGMENT_SHADER);

	float zNear = .01f;
	float zFar = 600.0f;

	data->shader = make_program({ vertex, geometry, fragment });
	glUseProgram(voxel_chunk::voxel_shader()->id);
	data->base =
		glm::perspective(glm::radians(60.0f), 1.0f, zNear, zFar)
		;

	uniform(*voxel_chunk::voxel_shader(), "num_lights", 8);

	std::vector<glm::vec3> lights;

	std::default_random_engine def;

	auto s = std::chrono::time_point_cast<std::chrono::nanoseconds>(clock.now());
	def.seed(s.time_since_epoch().count());
	std::uniform_int<> rand(0, data->size * 2);

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

	//lights.push_back(glm::vec3(0, 0, data->size + 2));

	//uniform(*voxel_chunk::voxel_shader(), "lights", lights);
	//data->voxel->subdivide(3);
	//for (int x = 0; x < data->size; x++)
	//{
	//	for (int y = 0; y < data->size; y++)
	//	{
	//		for (int z = 0; z < data->size; z++)
	//		{
	//			auto recentered_x = -x + data->size / 2;
	//			auto recentered_y = -y + data->size / 2;
	//			auto recentered_z = -z + data->size / 2;
	//			if (sqrt(recentered_x * recentered_x + recentered_y* recentered_y + recentered_z * recentered_z) <= data->size / 2
	//				&& (z * x / (y ? y : 1)) <= data->size / 2
	//				) // abs(x + y - z) <= 5

	//				voxel_data.on(x, y, z);
	//		}
	//	}
	//}


	glPointSize(5);
	//glUseProgram(0);

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDepthRange(0.0f, 1.0f);
	glClearDepth(1.0f);

	glutIdleFunc(&idle);
	glutDisplayFunc(&draw);

	glutPassiveMotionFunc(mouse);

	data->elapsed = glutGet(GLUT_ELAPSED_TIME);

	glutMainLoop();

	return 0;
}