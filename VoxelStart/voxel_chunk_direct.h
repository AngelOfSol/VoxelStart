#pragma once
#include <vector>
#include "nd_array.h"
#include "gl.h"
#include "shader_program.h"
struct gl_buffer
{
	gl_buffer();
	~gl_buffer();

	GLuint vao;
	GLuint vbo;

	void bind();
	void unbind();

};
class voxel_chunk_direct
{
public:
	using data = nd_array < signed int, 3 >;
	using gl_data = nd_array < float, 3 > ;

	voxel_chunk_direct(int width, int height, int depth);
	~voxel_chunk_direct();

	void set(bool val, int x, int y, int z);
	void on(int x, int y, int z) { this->set(true, x, y, z); };
	void off(int x, int y, int z) { this->set(false, x, y, z); };
	bool get(int x, int y, int z) const;

	void update();
	void draw(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 model_transform = glm::mat4());

	int width() const { return this->m_width; };
	int height() const { return this->m_height; };
	int depth() const { return this->m_depth; };

	glm::vec3 dimensions() const { return glm::vec3(this->width(), this->height(), this->depth()); };

	data::const_sub_view plane(int x) const;

	static const shader_program& shader();
private:

	unsigned int m_width;
	unsigned int m_depth;
	unsigned int m_height;

	void set_uniforms(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 extra_model_transform) const;

	static shader_program::ptr s_shader;

	bool m_changed;
	gl_data m_x_faces;
	gl_data m_y_faces;
	gl_data m_z_faces;

	data m_data;

	gl_buffer m_x_buffer, m_y_buffer, m_z_buffer;
};

