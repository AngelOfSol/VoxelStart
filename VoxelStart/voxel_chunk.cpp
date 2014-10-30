#include "voxel_chunk.h"
#include "debug.h"
#include "shader_data.h"
#include "uniform.h"
#include <initializer_list>
#include "timer.h"
#include <iostream>
voxel_chunk::voxel_chunk(int width, int height, int depth)
	: m_data(width, height, depth)
	, scale(1)
{
	glGenVertexArrays(1, &this->m_vao);
	glGenBuffers(1, &this->m_vbo);

	this->bind_buffers();
	this->fill_buffers();
	this->unbind_buffers();
}


voxel_chunk::~voxel_chunk()
{
	glDeleteVertexArrays(1, &this->m_vao);
	glDeleteBuffers(1, &this->m_vbo);
}

void voxel_chunk::update()
{
	if (this->changed)
	{
		/*
			if changed, schedule an update a thread to construct the contents of the new buffer
			, when that thread is done working, actually copy the buffer
			*/

		this->m_updates.schedule([](data_array data_copy)
		{
			update_result ret;
			ret.data = std::make_shared<gl_data>();
			ret.vertex_count = 0;

			auto center_x = -data_copy.dim_size<0>() / 2.0f + 0.5f;
			auto center_y = -data_copy.dim_size<1>() / 2.0f + 0.5f;
			auto center_z = -data_copy.dim_size<2>() / 2.0f + 0.5f;

			auto append = [&ret, center_x, center_y, center_z](int x, int y, int z)
			{
				ret.data->push_back(static_cast<GLfloat>(x + center_x));
				ret.data->push_back(static_cast<GLfloat>(y + center_y));
				ret.data->push_back(static_cast<GLfloat>(z + center_z));
			};

			for (int x = 0; x < data_copy.dim_size<0>(); x++)
			{
				auto x_plane = data_copy[x];
				for (int y = 0; y < data_copy.dim_size<1>(); y++)
				{
					auto y_axis = x_plane[y];
					for (int z = 0; z < data_copy.dim_size<2>(); z++)
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
									ret.data->emplace_back(static_cast<GLfloat>(i * 2 - 1));
									ret.data->emplace_back(static_cast<GLfloat>(0));
									ret.data->emplace_back(static_cast<GLfloat>(0));
									ret.vertex_count++;
								}
							}
							for (int i = 0; i < 2; i++)
							{
								if (!current_voxel.y[i])
								{
									append(x, y, z);
									ret.data->emplace_back(static_cast<GLfloat>(0));
									ret.data->emplace_back(static_cast<GLfloat>(i * 2 - 1));
									ret.data->emplace_back(static_cast<GLfloat>(0));
									ret.vertex_count++;
								}
							}

							for (int i = 0; i < 2; i++)
							{
								if (!current_voxel.z[i])
								{
									append(x, y, z);
									ret.data->emplace_back(static_cast<GLfloat>(0));
									ret.data->emplace_back(static_cast<GLfloat>(0));
									ret.data->emplace_back(static_cast<GLfloat>(i * 2 - 1));
									ret.vertex_count++;
								}
							}
						}
					}
				}
			}
			return ret;
		}, this->m_data);
		this->changed = false;
	}
	// if theres a new result, lets rebind
	if (this->m_updates.new_result())
	{
		
		auto& data = this->m_updates.get_result();

		this->m_active = data.vertex_count;

		this->bind_buffers();

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * data.data->size(), data.data->data());

		this->unbind_buffers();
	}
}

void voxel_chunk::draw(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 extra_model_transform)
{
	this->bind_buffers();
	this->set_uniforms(perspective, view_port, extra_model_transform);
	glDrawArrays(GL_POINTS, 0, this->m_active);
	this->unbind_buffers();
}
void voxel_chunk::on(int x, int y, int z)
{
	this->set(true, x, y, z);
}
void voxel_chunk::off(int x, int y, int z)
{
	this->set(false, x, y, z);
}
void voxel_chunk::set(bool b, int x, int y, int z)
{
	int change = b ? 1 : -1;

	auto& vox_ref = this->m_data[x][y][z];
	if (vox_ref.on != b)
	{
		this->changed = true;
		vox_ref.on = b;

		for (int i = -1; i < 2; i += 2)
		{
			if (x + i >= 0 && x + i < this->m_data.dim_size<0>())
			{
				auto& ref = this->m_data[x + i][y][z];

				ref.neighbors += change;
				ref.x[(-i + 1) / 2] = b;
			}
		}

		auto x_plane = this->m_data[x];
		for (int j = -1; j < 2; j += 2)
		{
			if (y + j >= 0 && y + j < this->m_data.dim_size<1>())
			{
				auto& ref = x_plane[y + j][z];

				ref.neighbors += change;
				ref.y[(-j + 1) / 2] = b;
			}
		}

		auto y_axis = x_plane[y];
		for (int k = -1; k < 2; k += 2)
		{
			if (z + k >= 0 && z + k < this->m_data.dim_size<2>())
			{
				auto& ref = y_axis[z + k];

				ref.neighbors += change;
				ref.z[(-k + 1) / 2] = b;
			}
		}


	}
}
bool voxel_chunk::get(int x, int y, int z) const
{
	return this->m_data[x][y][z].on;
}


void voxel_chunk::bind_buffers()
{
	glBindVertexArray(this->m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->m_vbo);
}

void voxel_chunk::fill_buffers()
{
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->width() * this->height() * this->depth() * 6, NULL, GL_DYNAMIC_DRAW);
	// let opengl know the first attribute is the position at 3 floats long starting at 0

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
	// let opengl know the second attribute is the position at 3 floats long starting at 3

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(GL_FLOAT)));

	// enable the attributes

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

}

void voxel_chunk::unbind_buffers()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void voxel_chunk::set_uniforms(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 extra_model_transform)
{
	glUseProgram(voxel_chunk::voxel_shader()->id);

	auto model = extra_model_transform * this->model_transform * glm::scale(glm::vec3(this->scale));
	auto mvp = perspective * view_port * model;
	uniform(*voxel_chunk::voxel_shader(), "mvp_transform", mvp);
	uniform(*voxel_chunk::voxel_shader(), "model_transform", model);
	uniform(*voxel_chunk::voxel_shader(), "normal_transform", glm::transpose(glm::inverse(model)));
}
const shader_program::ptr voxel_chunk::voxel_shader()
{
	if (voxel_chunk::s_voxel_shader == nullptr)
	{
		shader::ptr vertex = make_shader(shaders::voxel::vertex, GL_VERTEX_SHADER);
		shader::ptr geometry = make_shader(shaders::voxel::geometry, GL_GEOMETRY_SHADER);
		shader::ptr fragment = make_shader(shaders::voxel::fragment, GL_FRAGMENT_SHADER);

		voxel_chunk::s_voxel_shader = make_program({ vertex, geometry, fragment });
	}
	return voxel_chunk::s_voxel_shader;
}
void voxel_chunk::subdivide(int denom)
{
	data_array new_data(this->width() * denom, this->height() * denom, this->depth() * denom);
	for (int i = 0; i < this->width(); i++)
	{
		auto plane = this->m_data[i];
		for (int j = 0; j < this->height(); j++)
		{
			auto line = plane[j];
			for (int k = 0; k < this->depth(); k++)
			{
				auto ele = line[k];

				for (int copy_to_x = i * denom; copy_to_x < (i + 1) * denom; copy_to_x++)
				{
					auto copy_plane = new_data[copy_to_x];
					for (int copy_to_y = j * denom; copy_to_y < (j + 1) * denom; copy_to_y++)
					{
						auto copy_line = copy_plane[copy_to_y];
						for (int copy_to_z = k * denom; copy_to_z < (k + 1) * denom; copy_to_z++)
						{
							copy_line[copy_to_z] = ele;
						}
					}
				}
			}
		}
	}
	this->scale /= denom;
	this->changed = true;
	this->m_data = std::move(new_data);
	this->bind_buffers();
	this->fill_buffers();
	this->unbind_buffers();
}

int voxel_chunk::voxel::max_neighbors = 6;

shader_program::ptr voxel_chunk::s_voxel_shader = nullptr;