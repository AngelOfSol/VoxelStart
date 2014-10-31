#include "cut_voxel.h"
#include "debug.h"
struct overlap_data
{
	glm::vec3 start;
	glm::vec3 end;
};

overlap_data overlap_of(glm::vec3 position0, glm::vec3 dimension0, glm::vec3 position1, glm::vec3 dimension1)
{
	overlap_data ret;
	auto half_dim0 = dimension0 / 2.0f;
	auto half_dim1 = dimension1 / 2.0f;

	ret.start = glm::max(position0 - half_dim0, position1 - half_dim1);
	ret.end = glm::min(position0 + half_dim0, position1 + half_dim1);

	return ret;
}
void difference(glm::vec3 position0, voxel_chunk& minuend, glm::vec3 position1, const voxel_chunk& subtrahend)
{
	auto overlap = overlap_of(position0
		, glm::vec3{ minuend.width(), minuend.height(), minuend.depth() }
		, position1
		, glm::vec3{ subtrahend.width(), subtrahend.height(), subtrahend.depth() });
	
	auto get_minuend_voxel = [&minuend, position0](int x, int y, int z)
	{
		glm::vec3 pos{ x, y, z };
		return minuend.get(pos - position0 + minuend.dimensions() / 2.0f);
	};
	auto set_minuend_voxel = [&minuend, position0](bool val, int x, int y, int z)
	{
		glm::vec3 pos{ x, y, z };
		auto ans = pos - position0 + minuend.dimensions() / 2.0f;
		return minuend.set(val, pos - position0 + minuend.dimensions() / 2.0f);
	};
	auto get_subtrahend_voxel = [&subtrahend, position1](int x, int y, int z)
	{
		glm::vec3 pos{ x, y, z };
		return subtrahend.get(pos - position1 + subtrahend.dimensions() / 2.0f);
	};

	for (int i = static_cast<int>(overlap.start.x); i < static_cast<int>(overlap.end.x); i++)
	{
		for (int j = static_cast<int>(overlap.start.y); j < static_cast<int>(overlap.end.y); j++)
		{
			for (int k = static_cast<int>(overlap.start.z); k < static_cast<int>(overlap.end.z); k++)
			{
				set_minuend_voxel(get_minuend_voxel(i, j, k) && !get_subtrahend_voxel(i, j, k), i, j, k);
			}

		}
	}
} 