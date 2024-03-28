#version 150 core

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;


in Prim
{
	vec2 front;
	vec2 right;
	float boid_lenght;
	float boid_width;
} vsData[];

// out vec3 Color;

void main()
{
	gl_Position = gl_in[0].gl_Position;
    EmitVertex();
	gl_Position = gl_in[0].gl_Position - vsData[0].boid_lenght * vec4(vsData[0].front, 0.0f, 0.0f) + vsData[0].boid_width * vec4(vsData[0].right, 0.0f, 0.0f);
    EmitVertex();
	gl_Position = gl_in[0].gl_Position - vsData[0].boid_lenght * vec4(vsData[0].front, 0.0f, 0.0f) - vsData[0].boid_width * vec4(vsData[0].right, 0.0f, 0.0f);
    EmitVertex();

    EndPrimitive();
}
