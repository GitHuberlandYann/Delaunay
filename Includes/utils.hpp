#ifndef UTILS_HPP
# define UTILS_HPP

# define GLEW_STATIC
# include <GL/glew.h> // must be before glfw
# include "GLFW/glfw3.h"

# include <iostream>
# include <string>
# include <vector>
# include <array>

enum {
	POSATTRIB,
	RADATTRIB
};

std::string get_file_content( std::string file_name );
bool inRectangle( int posX, int posY, int rx, int ry, int width, int height );
float getPercent( const float value, const float range_start, const float range_end );
float gradient( float value, float start, float end, float grad_start, float grad_end );
int rgbaFromVec( std::array<float, 4> color );

std::string to_string_with_precision( const float value, const int precision = 2, const bool zero_allowed = true );

// shaders
GLuint createShaderProgram( std::string vertex, std::string geometry, std::string fragment );
void check_glstate( std::string str, bool displayDebug );

// textures
// void loadSubTextureArray( int layer, std::string texture_file );
void loadTextureShader( int index, GLuint texture, std::string texture_file );

#endif
