#pragma once
#include "nd_array.h"
#include "gl.h"
#include "shader_program.h"
#include "update.h"
class voxel_chunk
{

	struct voxel
	{
	public:
		static int max_neighbors;

		bool on;

		bool x[2]; // x[0] is this->x - 1 and x[1] is this->x + 1
		bool y[2]; // same as above
		bool z[2]; // same as above

		int neighbors;
		voxel()
			: on(false)
			, neighbors(0)
		{
			for (auto& b : x)
				b = false;
			for (auto& b : y)
				b = false;
			for (auto& b : z)
				b = false;
		}
	};
public:
	using data_array = nd_array<voxel, 3>;
	using gl_data = std::vector<GLfloat>;
	voxel_chunk(int width, int height, int depth);
	~voxel_chunk();

	bool changed;
	void update();
	void draw(glm::mat4 perspective, glm::mat4 view_port);
	
	void on(int x, int y, int z);
	void off(int x, int y, int z);
	void set(bool b, int x, int y, int z);
	bool get(int x, int y, int z)  const;

	int width() const { return this->m_data.dim_size<0>(); };
	int height() const { return this->m_data.dim_size<1>(); };
	int depth() const { return this->m_data.dim_size<2>(); };

	void subdivide(int denom);
	static const shader_program::ptr voxel_shader();

	double scale;

	glm::mat4 transform;
private:

	static shader_program::ptr s_voxel_shader;

	void bind_buffers();

	void fill_buffers();

	void unbind_buffers();

	void set_uniforms(glm::mat4 perspective, glm::mat4 view_port);



	data_array m_data;
	GLuint m_vao;
	GLuint m_vbo;

	int m_active;


	struct update_result
	{
		std::unique_ptr<gl_data> data;
		int vertex_count;
	};

	updater<update_result> m_updates;

};

