#include "cut_voxel.h"
#include "debug.h"

namespace voxel
{
	struct overlap_data
	{
		glm::vec3 start;
		glm::vec3 end;
	};

	overlap_data overlap_of(glm::vec3 position0, glm::vec3 dimension0, glm::vec3 position1, glm::vec3 dimension1)
	{
		overlap_data ret;

		ret.start = glm::max(position0, position1);
		ret.end = glm::min(position0 + dimension0, position1 + dimension1);

		return ret;
	}
	void difference(glm::vec3 position0, voxel_chunk_direct& minuend, glm::vec3 position1, const voxel_chunk_direct& subtrahend)
	{
		auto overlap = overlap_of(position0
			, glm::vec3{ minuend.width(), minuend.height(), minuend.depth() }
			, position1
			, glm::vec3{ subtrahend.width(), subtrahend.height(), subtrahend.depth() });

		auto get_minuend_voxel = [&minuend, position0](int x, int y, int z)
		{
			glm::vec3 pos{ x, y, z };
			glm::vec3 ds = pos - position0;
			return minuend.get(static_cast<int>(ds.x), static_cast<int>(ds.y), static_cast<int>(ds.z));
		};
		auto set_minuend_voxel = [&minuend, position0](bool val, int x, int y, int z)
		{

			glm::vec3 pos{ x, y, z };
			glm::vec3 ds = pos - position0;
			return minuend.set(val, static_cast<int>(ds.x), static_cast<int>(ds.y), static_cast<int>(ds.z));
		};
		auto get_subtrahend_voxel = [&subtrahend, position1](int x, int y, int z)
		{
			glm::vec3 pos{ x, y, z };
			glm::vec3 ds = pos - position1;
			return subtrahend.get(static_cast<int>(ds.x), static_cast<int>(ds.y), static_cast<int>(ds.z));
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
}