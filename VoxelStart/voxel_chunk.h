#pragma once
#include "nd_array.h"
#include "gl.h"
#include "shader_program.h"
#include <queue>
#include <future>
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

	struct voxel_updater_thread
	{
	public:
		struct result
		{
			std::unique_ptr<gl_data> data;
			int count;

			result()
			{

			}

			result(const result&) = delete;
			result(result&& r)
			{
				this->data = std::move(r.data);
				this->count = r.count;
			}
		};
		result update()
		{
			result ret;
			ret.data = std::make_unique<gl_data>();
			gl_data& data = *ret.data;
			ret.count = 0;

			auto center_x = -this->m_data.dim_size<0>() / 2.0f + 0.5f;
			auto center_y = -this->m_data.dim_size<1>() / 2.0f + 0.5f;
			auto center_z = -this->m_data.dim_size<2>() / 2.0f + 0.5f;

			auto append = [&data, center_x, center_y, center_z](int x, int y, int z)
			{
				data.push_back(static_cast<GLfloat>(x + center_x));
				data.push_back(static_cast<GLfloat>(y + center_y));
				data.push_back(static_cast<GLfloat>(z + center_z));
			};

			for (int x = 0; x < this->m_data.dim_size<0>(); x++)
			{
				auto x_plane = this->m_data[x];
				for (int y = 0; y < this->m_data.dim_size<1>(); y++)
				{
					auto y_axis = x_plane[y];
					for (int z = 0; z < this->m_data.dim_size<2>(); z++)
					{
						auto& current_voxel = y_axis[z];
						if (current_voxel.on && current_voxel.neighbors < voxel::max_neighbors)
						{
							// loop through x, y, z arrays and create faces

							for (int i = 0; i < 2; i++)
							{
								if (!current_voxel.x[i])
								{
									append(x, y, z);
									data.emplace_back(static_cast<GLfloat>(i * 2 - 1));
									data.emplace_back(static_cast<GLfloat>(0));
									data.emplace_back(static_cast<GLfloat>(0));
									ret.count++;
								}
							}
							for (int i = 0; i < 2; i++)
							{
								if (!current_voxel.y[i])
								{
									append(x, y, z);
									data.emplace_back(static_cast<GLfloat>(0));
									data.emplace_back(static_cast<GLfloat>(i * 2 - 1));
									data.emplace_back(static_cast<GLfloat>(0));
									ret.count++;
								}
							}

							for (int i = 0; i < 2; i++)
							{
								if (!current_voxel.z[i])
								{
									append(x, y, z);
									data.emplace_back(static_cast<GLfloat>(0));
									data.emplace_back(static_cast<GLfloat>(0));
									data.emplace_back(static_cast<GLfloat>(i * 2 - 1));
									ret.count++;
								}
							}
						}
					}
				}
			}

			return ret;
		}
	private:
		std::unique_ptr<gl_data> m_return;
		data_array m_data;
	};

	std::vector<voxel_updater_thread> m_updates;

};

