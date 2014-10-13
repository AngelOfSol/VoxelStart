#pragma once
#include "nd_array.h"
#include "gl.h"

class voxel_chunk
{
public:


	using ubyte = unsigned char;

	voxel_chunk(int width, int height, int depth);
	~voxel_chunk();

	bool changed;
	void update();
	void draw(GLenum);
	
	void on(int x, int y, int z);
	void off(int x, int y, int z);
	void set(bool b, int x, int y, int z);
	bool get(int x, int y, int z)  const;
private:

	void bind_buffers();

	void fill_buffers();

	void unbind_buffers();


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

	nd_array<voxel, 3> m_data;
	GLuint m_vao;
	GLuint m_vbo;

	int m_active;
};

