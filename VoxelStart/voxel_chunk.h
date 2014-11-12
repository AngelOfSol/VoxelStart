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
	voxel_chunk()
		: voxel_chunk(1, 1, 1)
	{

	}
	voxel_chunk(int width, int height, int depth);
	~voxel_chunk();

	bool changed;
	void update();
	void draw(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 extra_model_transform = glm::mat4()) const;

	void on(unsigned int x, unsigned int y, unsigned int z);
	void off(unsigned int x, unsigned int y, unsigned int z);
	void set(bool b, unsigned int x, unsigned int y, unsigned int z);
	bool get(unsigned int x, unsigned int y, unsigned int z)  const;

	template <typename Vector>
	void on(Vector v)
	{
		this->on(static_cast<unsigned int>(v.x), static_cast<unsigned int>(v.y), static_cast<unsigned int>(v.z));
	}
	template <typename Vector>
	void off(Vector v)
	{
		this->off(static_cast<unsigned int>(v.x), static_cast<unsigned int>(v.y), static_cast<unsigned int>(v.z));
	}
	template <typename Vector>
	void set(bool b, Vector v)
	{
		this->set(b, static_cast<unsigned int>(v.x), static_cast<unsigned int>(v.y), static_cast<unsigned int>(v.z));
	}
	template <typename Vector>
	bool get(Vector v) const
	{
		return this->get(static_cast<unsigned int>(v.x), static_cast<unsigned int>(v.y), static_cast<unsigned int>(v.z));
	}

	int width() const { return this->m_data.dim_size<0>(); };
	int height() const { return this->m_data.dim_size<1>(); };
	int depth() const { return this->m_data.dim_size<2>(); };

	void clear_and_resize();

	glm::vec3 dimensions() const { return glm::vec3{ this->width(), this->height(), this->depth() }; };

	void subdivide(unsigned int denom);
	static const shader_program::ptr voxel_shader();

	float scale;

	glm::mat4 model_transform;

	int remaining_tasks() const { return this->m_updates.remaining(); };
private:

	static shader_program::ptr s_voxel_shader;

	void bind_buffers() const;

	void fill_buffers();

	void unbind_buffers() const;

	void set_uniforms(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 extra_model_transform) const;



	data_array m_data;
	GLuint m_vao;
	GLuint m_vbo;

	int m_active;


	struct update_result
	{
		std::shared_ptr<gl_data> data;
		int vertex_count;
	};

	updater<update_result> m_updates;

};

