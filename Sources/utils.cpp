#include "utils.hpp"
#include <fstream>
#include <sstream>

std::string get_file_content( std::string file_name )
{
	std::ifstream indata (file_name.c_str());
	if (!indata.is_open()) {
		std::cerr << "Error\nCould not open " << file_name << std::endl;
		exit(1);
	}
	std::string data;
	std::string line;
	while (!indata.eof()) {
		std::getline( indata, line );
		data += line;
		if (!indata.eof())
			data += '\n';
	}
	indata.close();
	return (data);
}

bool inRectangle( int posX, int posY, int rx, int ry, int width, int height )
{
	// std::cout << "inRectangle pos " << posX << ", " << posY << std::endl;
	return (posX >= rx && posX <= rx + width && posY >= ry && posY <= ry + height);
}

float getPercent( const float value, const float range_start, const float range_end )
{
	// std::cout << "getPercent " << value << ", " << range_start << " - " << range_end << std::endl;
	if (value < range_start) return (0.0f);
	if (value > range_end) return (1.0f);
	return (value - range_start) / (range_end - range_start);
}

float gradient( float value, float start, float end, float grad_start, float grad_end )
{
	if (value <= start) {
		return (grad_start);
	} else if (value >= end) {
		return (grad_end);
	}

	float progress = (value - start) / (end - start);
	return (grad_start + progress * (grad_end - grad_start));
}

int rgbaFromVec( std::array<float, 4> color )
{
	int res = (static_cast<int>(color[0] * 255) & 0xFF) << 16;	// R
	res += (static_cast<int>(color[1] * 255) & 0xFF) << 8;		// G
	res += (static_cast<int>(color[2] * 255) & 0xFF);			// B
	res += (static_cast<int>(color[3] * 255) & 0xFF) << 24;		// A
	return (res);
}

std::string to_string_with_precision( const float value, const int precision, const bool zero_allowed )
{
    std::ostringstream out;
    out.precision(precision);
    out << std::fixed << value;
	if (zero_allowed) {
		return (std::move(out).str());
	}
	std::string res = std::move(out).str();
	size_t index = res.find('.'), i = 1;
	if (index == std::string::npos) {
		return (res);
	}
	while (res[index + i] == '0') ++i;
	if (res[index + i]) {
		return (res);
	}
	return (res.substr(0, index));
}

// ************************************************************************** //
//                                Shaders                                     //
// ************************************************************************** //

static void compile_shader( GLuint ptrShader, std::string name )
{
	glCompileShader(ptrShader);

    GLint status;
    glGetShaderiv(ptrShader, GL_COMPILE_STATUS, &status);
	if (status) {
		std::cout << name << " shader compiled successfully" << std::endl;
	} else {
		char buffer[512];
		glGetShaderInfoLog(ptrShader, 512, NULL, buffer);

		std::cerr << name << " shader did not compile, error log:" << std::endl << buffer << std::endl;
		exit(1);
	}
}

GLuint createShaderProgram( std::string vertex, std::string geometry, std::string fragment )
{
	std::string vertex_shader_data = get_file_content("Sources/Shaders/" + vertex + ".glsl");
	char *vertexSource = &vertex_shader_data[0];

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	compile_shader(vertexShader, vertex);

	GLuint geometryShader;
	if (geometry[0]) {
		std::string geometry_shader_data = get_file_content("Sources/Shaders/" + geometry + ".glsl");
		char *geometrySource = &geometry_shader_data[0];

		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometrySource, NULL);
		compile_shader(geometryShader, geometry);
	}

	GLuint fragmentShader;
	if (fragment[0]) {
		std::string fragment_shader_data = get_file_content("Sources/Shaders/" + fragment + ".glsl");
		char *fragmentSource = &fragment_shader_data[0];

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		compile_shader(fragmentShader, fragment);
	}

	// Combining shaders into a program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	if (geometry[0]) glAttachShader(shaderProgram, geometryShader);
	if (fragment[0]) glAttachShader(shaderProgram, fragmentShader);

	if (fragment[0]) glDeleteShader(fragmentShader);
	if (geometry[0]) glDeleteShader(geometryShader);
    glDeleteShader(vertexShader);

	return (shaderProgram);
}

void check_glstate( std::string str, bool displayDebug )
{
	GLenum error_check = glGetError();	
	if (error_check) {
		std::cerr << "glGetError set to " << error_check << " when trying to " << str << ", quitting now" << std::endl;
		exit(1);
	}
	if (displayDebug) {
		std::cout << str << std::endl;
	}
}

// ************************************************************************** //
//                                Textures                                    //
// ************************************************************************** //


#include "SOIL/SOIL.h"
typedef struct {
	unsigned char *content;
	int width;
	int height;
}				t_tex;
/*
void loadSubTextureArray( int layer, std::string texture_file )
{
	// load image
	t_tex img;
	img.content = SOIL_load_image(texture_file.c_str(), &img.width, &img.height, 0, SOIL_LOAD_RGBA);
	if (!img.content) {
		std::cerr << "failed to load image " << texture_file << " because:" << std::endl << SOIL_last_result() << std::endl;
		exit(1);
	}

	if (img.width != 300 || img.height != 300) {
		std::cerr << texture_file << ": image size not 300x300 but " << img.width << "x" << img.height << std::endl;
		exit(1);
	}
	// Upload pixel data.
	// The first 0 refers to the mipmap level (level 0, since there's only 1)
	// The following 2 zeroes refers to the x and y offsets in case you only want to specify a subrectangle.
	// 300x300 size of rect, 1 = depth of layer
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, img.width, img.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, img.content);
			
	// set settings for texture wraping and size modif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SOIL_free_image_data(img.content);

	check_glstate("Succesfully loaded " + texture_file + " to shader", true);
}*/

void loadTextureShader( int index, GLuint texture, std::string texture_file )
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, texture);

	// load image
	t_tex img;
	img.content = SOIL_load_image(texture_file.c_str(), &img.width, &img.height, 0, SOIL_LOAD_RGBA);
	if (!img.content) {
		std::cerr << "failed to load image " << texture_file << " because:" << std::endl << SOIL_last_result() << std::endl;
		exit(1);
	}

	// load image as texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, img.content);
			
	// set settings for texture wraping and size modif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_NEAREST because pixel art, otherwise GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	SOIL_free_image_data(img.content);

	check_glstate("Succesfully loaded " + texture_file + " to shader", true);
}
