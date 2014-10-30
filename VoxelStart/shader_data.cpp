#include "shader_data.h"


namespace shaders
{
namespace voxel
{

	std::string vertex = R"(
#version 410

layout (location = 0) in vec3 position;

layout (location = 1) in vec3 normal;

out vec4 gNormal;

void main()
{
	gl_Position = vec4(position, 1);
	gNormal = vec4(normal, 1);
}

)";

	std::string geometry = R"(

#version 410

const mat4 top_left = mat4(
0.5, 0.5, 0.5, 0
, 0.5 , 0.5, 0.5, 0
, 0.5, 0.5, 0.5, 0
, 0, 0, 0, 1
);
const mat4 top_right = mat4(
0.5, 0.5, -0.5, 0
, 0.5 , 0.5, -0.5, 0
, 0.5, -0.5, 0.5, 0
, 0, 0, 0, 1
);
const mat4 bottom_left = mat4(
0.5, -0.5, 0.5, 0
, -0.5 , 0.5, 0.5, 0
, -0.5, 0.5, 0.5, 0
, 0, 0, 0, 1
);
const mat4 bottom_right = mat4(
0.5, -0.5, -0.5, 0
, -0.5 , 0.5, -0.5, 0
, -0.5, -0.5, 0.5, 0
, 0, 0, 0, 1
);


in vec4 gNormal[];
out vec4 fNormal;
out vec4 camera_position;

uniform mat4 mvp_transform;
uniform mat4 model_transform;
uniform mat4 normal_transform;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

void main() 
{
	vec4 base = gl_in[0].gl_Position;
	vec4 normal = gNormal[0];

	vec4 bottom_left_vert = (base + bottom_left * normal);
	vec4 top_left_vert = (base + top_left * normal);
	vec4 bottom_right_vert = (base + bottom_right * normal);
	vec4 top_right_vert = (base + top_right * normal);
	
	vec4 actual_normal = normal_transform * normal;

	gl_Position = mvp_transform * bottom_left_vert;
	camera_position = model_transform * bottom_left_vert;
	fNormal = actual_normal;
	EmitVertex();
	
	gl_Position = mvp_transform * top_left_vert;
	camera_position = model_transform * top_left_vert;
	fNormal = actual_normal;
	EmitVertex();
	
	gl_Position = mvp_transform * bottom_right_vert;
	camera_position = model_transform * bottom_right_vert;
	fNormal = actual_normal;
	EmitVertex();
	

	gl_Position = mvp_transform * top_right_vert;
	camera_position = model_transform * top_right_vert;
	fNormal = actual_normal;
	EmitVertex();
	

	EndPrimitive();
}


)";

std::string fragment = R"(
#version 410

uniform vec3 lights[100];
uniform int num_lights;

in vec4 fNormal;
in vec4 camera_position;

float calc_light(vec4 light)
{
	vec4 diff = light - camera_position;
	float attenuation = length(diff);
	float index_of_refrac = (dot(fNormal, normalize(diff)), 0 , 1);

	return index_of_refrac / (attenuation);
}

void main(void) 
{
	float mult = 0.0;

	for(int i = 0; i < num_lights; i++)
	{
		mult += 5 * calc_light(vec4(lights[i], 1));
	}

	gl_FragColor = vec4(0.6, 0.6, 0.8, 1) * (mult);
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