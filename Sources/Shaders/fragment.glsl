#version 150 core

in float greyScale;

out vec4 outColor;

void main()
{
	outColor = vec4(greyScale, greyScale, greyScale, 1.0);
}
