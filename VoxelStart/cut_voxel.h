#include "gl.h"
#include "voxel_chunk.h"
#include "voxel_chunk_direct.h"

namespace voxel
{
	//void difference(glm::vec3 position0, voxel_chunk& minuend, glm::vec3 position1, const voxel_chunk& subtrahend);
	void difference(glm::vec3 position0, voxel_chunk_direct& minuend, glm::vec3 position1, const voxel_chunk_direct& subtrahend);
}