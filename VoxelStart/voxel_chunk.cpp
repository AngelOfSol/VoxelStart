#include "voxel_chunk.h"
#include "debug.h"
#include <initializer_list>
voxel_chunk::voxel_chunk(int width, int height, int depth)
	: m_data(width, height, depth)
{
	glGenVertexArrays(1, &this->m_vao);
	glGenBuffers(1, &this->m_vbo);
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
		this->bind_buffers();

		this->fill_buffers();

		this->unbind_buffers();

		this->changed = false;
	}
}

void voxel_chunk::draw(GLenum mode)
{
	this->bind_buffers();
	glDrawArrays(mode, 0, this->m_active);
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
	std::vector<GLfloat> data;
	int count = 0;

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
							count++;
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
							count++;
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
							count++;
						}
					}
				}
			}
		}
	}

	this->m_active = count;
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_STATIC_DRAW);

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

int voxel_chunk::voxel::max_neighbors = 6;