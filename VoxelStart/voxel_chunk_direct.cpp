#include "voxel_chunk_direct.h"
#include "shader_data.h"
#include "uniform.h"
voxel_chunk_direct::voxel_chunk_direct(int width, int height, int depth)
	: m_data(width, height, depth)
	, m_x_faces(width + 1, height, depth)
	, m_y_faces(width, height + 1, depth)
	, m_z_faces(width, height, depth + 1)
	, m_width(width)
	, m_height(height)
	, m_depth(depth)
{

	this->m_x_buffer.bind();

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * (this->width() + 1) * this->height() * this->depth(), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(GL_FLOAT), (void*)(0));

	glEnableVertexAttribArray(0);

	this->m_y_buffer.bind();

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * this->width() * (this->height() + 1) * this->depth(), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(GL_FLOAT), (void*)(0));

	glEnableVertexAttribArray(0);

	this->m_z_buffer.bind();

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * this->width() * this->height() * (this->depth() + 1), NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(GL_FLOAT), (void*)(0));

	glEnableVertexAttribArray(0);

	this->m_z_buffer.unbind();

	this->m_changed = true;
}


voxel_chunk_direct::~voxel_chunk_direct()
{
}


void voxel_chunk_direct::set(bool val, int x, int y, int z)
{
	if (val != this->get(x, y, z))
	{
		this->m_changed = true;
		this->m_data[x][y][z] = val;

		auto mult = val ? 1 : -1;

		this->m_x_faces[x][y][z] -= mult;
		this->m_y_faces[x][y][z] -= mult;
		this->m_z_faces[x][y][z] -= mult;

		this->m_x_faces[x + 1][y][z] += mult;
		this->m_y_faces[x][y + 1][z] += mult;
		this->m_z_faces[x][y][z + 1] += mult;
	}
}
bool voxel_chunk_direct::get(int x, int y, int z) const
{
	return this->m_data[x][y][z];
}

auto voxel_chunk_direct::plane(int x) const -> data::const_sub_view
{
	return this->m_data[x];
}

void voxel_chunk_direct::update()
{
	if (this->m_changed)
	{
		this->m_changed = false;

		this->m_x_buffer.bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GL_FLOAT) * this->m_x_faces.size(), this->m_x_faces.data());

		this->m_y_buffer.bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GL_FLOAT) * this->m_y_faces.size(), this->m_y_faces.data());

		this->m_z_buffer.bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GL_FLOAT) * this->m_z_faces.size(), this->m_z_faces.data());
		this->m_z_buffer.unbind();
	}
}
void voxel_chunk_direct::draw(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 model_transform)
{
	this->set_uniforms(perspective, view_port, model_transform);
	this->m_x_buffer.bind();
	uniform(voxel_chunk_direct::shader(), "direction", glm::vec3{ 1, 0, 0 });
	uniform(voxel_chunk_direct::shader(), "array_dimensions", glm::vec3{ this->width() + 1, this->height(), this->depth() });
	glDrawArrays(GL_POINTS, 0, this->m_x_faces.size());

	this->m_y_buffer.bind();
	uniform(voxel_chunk_direct::shader(), "direction", glm::vec3{ 0, 1, 0 });
	uniform(voxel_chunk_direct::shader(), "array_dimensions", glm::vec3{ this->width(), this->height() + 1, this->depth() });
	glDrawArrays(GL_POINTS, 0, this->m_y_faces.size());

	this->m_z_buffer.bind();
	uniform(voxel_chunk_direct::shader(), "direction", glm::vec3{ 0, 0, 1 });
	uniform(voxel_chunk_direct::shader(), "array_dimensions", glm::vec3{ this->width(), this->height(), this->depth() + 1 });
	glDrawArrays(GL_POINTS, 0, this->m_z_faces.size());

	this->m_z_buffer.unbind();
}

void voxel_chunk_direct::set_uniforms(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 extra_model_transform) const
{
	glUseProgram(voxel_chunk_direct::shader().id);

	auto model = extra_model_transform;
	auto mvp = perspective * view_port * model;
	uniform(voxel_chunk_direct::shader(), "mvp_transform", mvp);
	uniform(voxel_chunk_direct::shader(), "model_transform", model);
	uniform(voxel_chunk_direct::shader(), "normal_transform", glm::transpose(glm::inverse(model)));
}

const shader_program& voxel_chunk_direct::shader()
{
	if (voxel_chunk_direct::s_shader == nullptr)
	{
		shader::ptr vertex = make_shader(shaders::voxel_direct::vertex, GL_VERTEX_SHADER);
		shader::ptr geometry = make_shader(shaders::voxel_direct::geometry, GL_GEOMETRY_SHADER);
		shader::ptr fragment = make_shader(shaders::voxel_direct::fragment, GL_FRAGMENT_SHADER);

		voxel_chunk_direct::s_shader = make_program({ vertex, geometry, fragment });
	}
	return *voxel_chunk_direct::s_shader;
}

shader_program::ptr voxel_chunk_direct::s_shader = nullptr;

gl_buffer::gl_buffer()
{
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
}
gl_buffer::~gl_buffer()
{
	glDeleteBuffers(1, &this->vbo);
	glDeleteVertexArrays(1, &this->vao);
}
void gl_buffer::bind()
{
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
}
void gl_buffer::unbind()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}