#pragma once
#include "voxel_chunk.h"
#include "nd_array.h"
class voxel_model
{
public:
	using data = nd_array <voxel_chunk, 3> ;
	voxel_model(int chunk_size, glm::vec3 number_of_voxel_chunks);
	~voxel_model();
	void draw(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 extra_model_transform = glm::mat4());
	void difference(const voxel_model & subtrahend);
	void update();
	template <typename Fn>
	void graph(Fn equation)
	{
		this->reset();
		for (int i = 0; i < this->width; i++)
		{
			for (int j = 0; j < this->height(); j++)
			{
				for (int k = 0; k < this->depth(); k++)
				{
					this->set(equation(i, j, k), i, j, k);
				}
			}
		}
	} 
	void reset();
	void set(bool value, int x, int y, int z);
	bool get(int x, int y, int z) const;
	void on(int x, int y, int z);
	void off(int x, int y, int z);

	int width() const { return this->m_chunk_size * this->m_voxels.dim_size<0>(); };
	int height() const { return this->m_chunk_size * this->m_voxels.dim_size<1>(); };
	int depth() const { return this->m_chunk_size * this->m_voxels.dim_size<2>(); };

	glm::vec3 position;
private:
	const int m_chunk_size;
	data m_voxels;
	struct voxel_iterator
	{
		voxel_iterator(voxel_model& parent, int x, int y, int z)
			: m_parent(parent)
			, m_x(x)
			, m_y(y)
			, m_z(z)
			, plane(parent.m_voxels[this->m_x])
			, line(this->plane[this->m_y])
		{

		}

		voxel_chunk& operator*()
		{
			return this->line[this->m_z];
		}
		voxel_chunk* operator->()
		{
			return &this->line[this->m_x];
		}
		bool operator !=(const voxel_iterator& rhs)
		{
			return !(rhs.m_x == this->m_x && rhs.m_y == this->m_y && rhs.m_z == this->m_z);
		}

		voxel_iterator& operator++()
		{
			this->m_z++;
			if (this->m_z >= this->m_parent.m_voxels.dim_size<2>())
			{
				this->m_z = 0;
				this->m_y++;

				if (this->m_y >= this->m_parent.m_voxels.dim_size<1>())
				{
					this->m_y = 0;
					this->m_x++;
					this->plane = this->m_parent.m_voxels[this->m_x];
				}

				this->line = this->plane[this->m_y];
			}
			return *this;
		}
		int x() const { return this->m_x; };
		int y() const { return this->m_y; };
		int z() const { return this->m_z; };
		glm::vec3 vec() const { return glm::vec3{this->x(), this->y(), this->z()}; };
	private:
		voxel_model& m_parent;
		int m_x;
		int m_y;
		int m_z;
		data::sub_view plane;
		data::sub_view::sub_view line;
	};
	struct const_voxel_iterator
	{
		const_voxel_iterator(const voxel_model& parent, int x, int y, int z)
			: m_parent(parent)
			, m_x(x)
			, m_y(y)
			, m_z(z)
			, plane(parent.m_voxels[this->m_x])
			, line(this->plane[this->m_y])
		{

		}

		const voxel_chunk& operator*()
		{
			return this->line[this->m_z];
		}
		const voxel_chunk* operator->()
		{
			return &this->line[this->m_x];
		}

		bool operator !=(const_voxel_iterator rhs)
		{
			return !(rhs.m_x == this->m_x && rhs.m_y == this->m_y && rhs.m_z == this->m_z);
		}

		const_voxel_iterator& operator++()
		{
			this->m_z++;
			if (this->m_z > this->m_parent.m_voxels.dim_size<2>())
			{
				this->m_z = 0;
				this->m_y++;

				if (this->m_y > this->m_parent.m_voxels.dim_size<1>())
				{
					this->m_y = 0;
					this->m_x++;
					this->plane = this->m_parent.m_voxels[this->m_x];
				}

				this->line = this->plane[this->m_y];
			}
			return *this;
		}
		int x() const { return this->m_x; };
		int y() const { return this->m_y; };
		int z() const { return this->m_z; };
		glm::vec3 vec() const { return glm::vec3{ this->x(), this->y(), this->z() }; };
	private:
		const voxel_model& m_parent;
		int m_x;
		int m_y;
		int m_z;
		data::const_sub_view plane;
		data::const_sub_view::sub_view line;
	};

	voxel_iterator begin()
	{
		return voxel_iterator(*this, 0, 0, 0);
	}
	voxel_iterator end()
	{
		return voxel_iterator(*this, this->m_voxels.dim_size<0>(), 0, 0);
	}
	const_voxel_iterator begin() const
	{
		return const_voxel_iterator(*this, 0, 0, 0);
	}
	const_voxel_iterator end() const
	{
		return const_voxel_iterator(*this, this->m_voxels.dim_size<0>(), 0, 0);
	}
};

