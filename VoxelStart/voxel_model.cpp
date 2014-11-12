#include "voxel_model.h"
#include "cut_voxel.h"
#include "debug.h"
voxel_model::voxel_model(int chunk_size, glm::vec3 number_of_voxel_chunks)
	: m_voxels(static_cast<int>(number_of_voxel_chunks.x), static_cast<int>(number_of_voxel_chunks.y), static_cast<int>(number_of_voxel_chunks.z))
	, m_chunk_size(chunk_size)
{
}

voxel_model::~voxel_model()
{
}

void voxel_model::draw(glm::mat4 perspective, glm::mat4 view_port, glm::mat4 extra_model_transform)
{
	for (auto voxel_iter = this->begin(); voxel_iter != this->end(); ++voxel_iter)
	{
		voxel_iter->draw(perspective, view_port, extra_model_transform * glm::translate(this->position) * glm::translate(voxel_iter.vec()));
	}
}
void voxel_model::difference(const voxel_model & subtrahend)
{
	for (auto voxel_iter = this->begin(); voxel_iter != this->end(); ++voxel_iter)
	{
		auto voxel_pos = voxel_iter.vec() + this->position;
		for (auto sub_voxel_iter = subtrahend.begin(); sub_voxel_iter != subtrahend.end(); ++sub_voxel_iter)
		{
			voxel::difference(voxel_pos, *voxel_iter, sub_voxel_iter.vec() + subtrahend.position, *sub_voxel_iter);
		}
	}
}
void voxel_model::update()
{
	for (auto& voxel : *this)
	{
		voxel.update();
	}
}
void voxel_model::reset()
{
	for (auto voxel_iter = this->begin(); voxel_iter != this->end(); ++voxel_iter)
	{
		voxel_iter->clear_and_resize();
	}
}
void voxel_model::set(bool value, int x, int y, int z)
{
	auto& vox_ref = this->m_voxels[x / this->m_chunk_size][y / this->m_chunk_size][z / this->m_chunk_size];
	if (vox_ref.width() == 1)
	{
		vox_ref.subdivide(this->m_chunk_size);
		vox_ref.scale = 1;
	}
	vox_ref.set(value, x % this->m_chunk_size, y % this->m_chunk_size, z % this->m_chunk_size);
}
bool voxel_model::get(int x, int y, int z) const
{
	auto& vox_ref = this->m_voxels[x / this->m_chunk_size][y / this->m_chunk_size][z / this->m_chunk_size];
	if (vox_ref.width() == 1)
		return vox_ref.get(0, 0, 0);
	return vox_ref.get(x % this->m_chunk_size, y % this->m_chunk_size, z % this->m_chunk_size);
}
void voxel_model::on(int x, int y, int z)
{
	this->set(true, x, y, z);
}
void voxel_model::off(int x, int y, int z)
{
	this->set(false, x, y, z);
}