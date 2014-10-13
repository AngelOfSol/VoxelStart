#include "shader_data.h"


namespace shaders
{
namespace voxel
{

	std::string vertex = R"(
#version 410

uniform mat4 transform;
layout (location = 0) in vec3 position;
out vec4 color;
void main()
{
	gl_Position = vec4((transform * vec4(position, 1)).xyz, 1);
	color = vec4(1, 1, 0.6, 1) / distance(vec3(-0), position) * 20;
}
)";

	std::string fragment = R"(
#version 410
in vec4 color;
void main(void) 
{
	gl_FragColor = color;
}
)";
}

namespace image
{
std::string vertex = R"(
#version 410
void main()
{
	gl_Position = vec4(0, 0, 0, 1);
}
)";

std::string geometry = R"(
#version 410

out vec2 fTexel;

uniform mat4 transform;

uniform vec2 frameOffset;
uniform vec2 frameSize;
uniform vec2 imageSize;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

void main() 
{

	gl_Position = transform * (gl_in[0].gl_Position + vec4(0.5 * frameSize.x, 0.5 * frameSize.y, 0, 0));
	fTexel = (frameOffset + vec2(frameSize.x, 0)) / imageSize;
	EmitVertex();

	gl_Position = transform * (gl_in[0].gl_Position + vec4(-0.5 * frameSize.x, 0.5 * frameSize.y, 0, 0));
	fTexel = (frameOffset) / imageSize;
	EmitVertex();

	gl_Position = transform * (gl_in[0].gl_Position + vec4(0.5 * frameSize.x, -0.5 * frameSize.y, 0, 0));
	fTexel = (frameOffset + frameSize) / imageSize;
	EmitVertex();
	
	gl_Position = transform * (gl_in[0].gl_Position + vec4(-0.5 * frameSize.x, -0.5 * frameSize.y, 0, 0));
	fTexel = (frameOffset + vec2(0, frameSize.y)) / imageSize;
	EmitVertex();


	EndPrimitive();
}
)";
std::string fragment = R"(
#version 410

varying vec2 fTexel;

uniform sampler2D image;

void main(void) 
{
	gl_FragColor = vec4(texture(image, fTexel));
}
)";
}
}