#include <iostream>
#include "gl.h"
#include <memory>
#include "voxel_chunk.h"
#include "nd_array.h"
#include "shader_program.h"
#include <initializer_list>
#include "shader_data.h"
#include "uniform.h"
#include <glm/glm/gtc/quaternion.hpp>
#include <chrono>
using std::unique_ptr;
using std::make_unique;

struct Data
{
	std::unique_ptr<voxel_chunk> voxel;
	shader_program::ptr shader;

	glm::mat4 transform;
	glm::mat4 base;
	glm::mat4 rotate;
	GLfloat angle;
	const int size = 300;
	int elapsed;
	glm::vec2 mouse;
};

std::unique_ptr<Data> data;

void idle()
{
	auto change = glutGet(GLUT_ELAPSED_TIME) - data->elapsed;
	data->elapsed += change;
	data->angle += 0.0005f * change;

	auto rotate_x = glm::fquat();

	glm::vec2 middle = glm::vec2{ 400, 400 };
	auto diff = data->mouse - middle;
	auto invert = glm::vec2{ -diff.y, diff.x };
	data->rotate *= glm::rotate(glm::radians( change * .1f), glm::vec3(invert, 0));

	data->transform = data->base
		* data->rotate
		* glm::translate(glm::vec3(-data->size / 2.0f));

	//data->transform = data->base * glm::translate(glm::vec3(diff.x, -diff.y, 0) * 0.1f) * glm::rotate(glm::radians(45.0f), glm::vec3{ 0.0f, 1.0f, 0.0f }) * glm::rotate(data->angle, glm::vec3{ 1.0f, 0.0f, 0.0f }) *
	//	glm::translate(glm::vec3(-data->size / 2.0f));
	uniform(*data->shader, "transform", data->transform);
	glutPostRedisplay();
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	data->voxel->draw(GL_POINTS);
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
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

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
					&& abs(recentered_x - recentered_y) <= data->size / 2
					&& abs(recentered_y - recentered_z) <= data->size / 2
					&& abs(recentered_z - recentered_x) <= data->size / 2) // abs(x + y - z) <= 5
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
	shader::ptr fragment = make_shader(shaders::voxel::fragment, GL_FRAGMENT_SHADER);

	float zNear = .01;
	float zFar = 20;

	data->shader = make_program({ vertex, fragment });
	glUseProgram(data->shader->id);
	data->base =
		glm::perspective(glm::radians(70.0f), 1.0f, zNear, zFar) 
		* glm::scale(glm::vec3(0.07f  / data->size * 5.0f))
		;


	glPointSize(5);
	//glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glClearDepth(1.0f);

	glutIdleFunc(&idle);
	glutDisplayFunc(&draw);

	glutPassiveMotionFunc(mouse);

	data->elapsed = glutGet(GLUT_ELAPSED_TIME);

	glutMainLoop();

	return 0;
}