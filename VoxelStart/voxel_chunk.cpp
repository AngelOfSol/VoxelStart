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
	auto& vox_ref = this->m_data[x][y][z];
	if (!vox_ref.on)
	{
		this->changed = true;
		vox_ref.on = true;

		auto inBounds = [this](int x, int y, int z) -> bool
		{
			if (x < 0 || y < 0 || z < 0)
				return false;

			if (x >= this->m_data.dim_size<0>() || y >= this->m_data.dim_size<1>() || z >= this->m_data.dim_size<2>())
				return false;

			return true;
		};


		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				for (int k = -1; k < 2; k++)
				{
					if ((i != 0 || j != 0 || k != 0) && inBounds(x + i, y + j, z +k))
					{
						this->m_data[x + i][y + j][z + k].neighbors += 1;
					}
				}
			}
		}
		// increment members
	}
}
void voxel_chunk::off(int x, int y, int z)
{
	auto& vox_ref = this->m_data[x][y][z];
	if (vox_ref.on)
	{
		this->changed = true;
		vox_ref.on = false;
		auto inBounds = [this](int x, int y, int z) -> bool
		{
			if (x < 0 || y < 0 || z < 0)
				return false;

			if (x >= this->m_data.dim_size<0>() || y >= this->m_data.dim_size<1>() || z >= this->m_data.dim_size<2>())
				return false;

			return true;
		};


		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				for (int k = -1; k < 2; k++)
				{
					if ((i != 0 || j != 0 || k != 0) && inBounds(x + i, y + j, z + k))
					{
						this->m_data[x + i][y + j][z + k].neighbors -= 1;
					}
				}
			}
		}
		// decrement members
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

	for (int z = 0; z < this->m_data.dim_size<2>(); z++)
	{
		for (int x = 0; x < this->m_data.dim_size<0>(); x++)
		{
			for (int y = 0; y < this->m_data.dim_size<1>(); y++)
			{
				if (this->m_data[x][y][z].on && this->m_data[x][y][z].neighbors < 26)
				{
					data.emplace_back(static_cast<GLfloat>(x));
					data.emplace_back(static_cast<GLfloat>(y));
					data.emplace_back(static_cast<GLfloat>(z));

					count++;
				}
			}
		}
	}

	this->m_active = count;

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_STATIC_DRAW);

	// let opengl know the first attribute is the position at 3 floats long starting at 0

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	// enable the attributes

	glEnableVertexAttribArray(0);

}

void voxel_chunk::unbind_buffers()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}