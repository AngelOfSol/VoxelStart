#include "gl.h"
#include "voxel_chunk.h"
void difference(glm::vec3 position0, voxel_chunk& minuend, glm::vec3 position1, const voxel_chunk& subtrahend)
{
	auto overlap = overlap_of(position0
		, glm::vec3{ minuend.width(), minuend.height(), minuend.depth() }
		, position1
		, glm::vec3{ subtrahend.width(), subtrahend.height(), subtrahend.depth() });
	/*
		
	*/
	auto get_minuend_voxel = [&minuend, position0](int x, int y, int z)
	{
		glm::vec3 pos{ x, y, z };
		return minuend.get(pos - position0 + minuend.dimensions() / 2.0f);
	};
	auto set_minuend_voxel = [&minuend, position0](bool val, int x, int y, int z)
	{
		glm::vec3 pos{ x, y, z };
		return minuend.set(val, pos - position0 + minuend.dimensions() / 2.0f);
	};
	auto get_subtrahend_voxel = [&subtrahend, position1](int x, int y, int z)
	{
		glm::vec3 pos{ x, y, z };
		return subtrahend.get(pos - position1 + subtrahend.dimensions() / 2.0f);
	};

	for (int i = overlap.start.x; i < overlap.end.x; i++)
	{
		for (int j = overlap.start.y; j < overlap.end.y; j++)
		{
			for (int k = overlap.start.z; k < overlap.end.z; k++)
			{
				set_minuend_voxel(get_minuend_voxel(i, j, k) && !get_subtrahend_voxel(i, j, k), i, j, k);
			}
		}
	}
}

struct overlap_data
{
	glm::vec3 start;
	glm::vec3 duration;
	glm::vec3 end;
};

overlap_data overlap_of(glm::vec3 position0, glm::vec3 dimension0, glm::vec3 position1, glm::vec3 dimension1)
{
	overlap_data ret;

	ret.duration = dimension0 / 2.0f + dimension1 / 2.0f - (glm::abs(position1 - position0));
	ret.start = glm::min(position1, position0);
	ret.end = ret.start + ret.duration;

	return ret;
}