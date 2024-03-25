#include "Text.hpp"

Text::Text( void ) : _texture(0)
{
}

Text::~Text( void )
{
	std::cout << "Destructor of Text called" << std::endl;
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);

	if (_texture) {
		glDeleteTextures(1, &_texture);
	}
	glDeleteProgram(_shaderProgram);
}


// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

void Text::setup_shader( void )
{
	_shaderProgram = createShaderProgram("gui_vertex", "", "gui_fragment");

	glBindFragDataLocation(_shaderProgram, 0, "outColor");

	glBindAttribLocation(_shaderProgram, TEXT::SPECATTRIB, "specifications");
	glBindAttribLocation(_shaderProgram, TEXT::COLATTRIB, "color");
	glBindAttribLocation(_shaderProgram, TEXT::POSATTRIB, "position");

	glLinkProgram(_shaderProgram);
	glUseProgram(_shaderProgram);

	check_glstate("text_Shader program successfully created", true);
}

void Text::setup_communication_shader( void )
{
	_uniWidth = glGetUniformLocation(_shaderProgram, "win_width");
	_uniHeight = glGetUniformLocation(_shaderProgram, "win_height");

	check_glstate("\nCommunication with text shader program successfully established", false);

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	check_glstate("text VAO and VBO", false);
}

void Text::load_texture( void )
{
	glUseProgram(_shaderProgram);

	glGenTextures(1, &_texture);

	loadTextureShader(1, _texture, "Resources/asciiAtlas.png");
	glUniform1i(glGetUniformLocation(_shaderProgram, "asciiAtlas"), 1); // sampler2D #index in fragment shader
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

GLuint Text::start( void )
{
	setup_shader();
	setup_communication_shader();
	load_texture();
	return (_shaderProgram);
}

void Text::setWindowSize( int width, int height )
{
	glUseProgram(_shaderProgram);

	glUniform1i(_uniWidth, width);
	glUniform1i(_uniHeight, height);
}

void Text::addTriangle( int spec, std::array<int, 2> a, std::array<int, 2> b, std::array<int, 2> c, int color )
{
	_texts.push_back(spec + (1 << 8) + (0 << 9));
	_texts.push_back(color);
	_texts.push_back(a[0]);
	_texts.push_back(a[1]);
	_texts.push_back(spec + (1 << 8) + (1 << 9));
	_texts.push_back(color);
	_texts.push_back(b[0]);
	_texts.push_back(b[1]);
	_texts.push_back(spec + (0 << 8) + (1 << 9));
	_texts.push_back(color);
	_texts.push_back(c[0]);
	_texts.push_back(c[1]);
}

void Text::addQuads( int spec, int posX, int posY, int width, int height, int color )
{
	_texts.push_back(spec + (0 << 8) + (0 << 9));
	_texts.push_back(color);
	_texts.push_back(posX);
	_texts.push_back(posY);
	_texts.push_back(spec + (1 << 8) + (0 << 9));
	_texts.push_back(color);
	_texts.push_back(posX + width);
	_texts.push_back(posY);
	_texts.push_back(spec + (0 << 8) + (1 << 9));
	_texts.push_back(color);
	_texts.push_back(posX);
	_texts.push_back(posY + height);

	_texts.push_back(spec + (1 << 8) + (0 << 9));
	_texts.push_back(color);
	_texts.push_back(posX + width);
	_texts.push_back(posY);
	_texts.push_back(spec + (1 << 8) + (1 << 9));
	_texts.push_back(color);
	_texts.push_back(posX + width);
	_texts.push_back(posY + height);
	_texts.push_back(spec + (0 << 8) + (1 << 9));
	_texts.push_back(color);
	_texts.push_back(posX);
	_texts.push_back(posY + height);
}

int Text::textWidth( int font_size, std::string str, int limit )
{
	int res = 0;
	for (size_t i = 0, charLine = 0; str[i]; ++i, ++charLine) {
		if (limit != -1 && static_cast<int>(i) >= limit) return (res);
		switch (str[i]) {
			case '\n':
				break ;
			case '\t':
				charLine += 4 - (charLine & 3);
				res = charLine * font_size;
				break ;
			case 'i':
			case '.':
			case ':':
			case '!':
			case '\'':
			case ',':
			case ';':
			case '|':
			case '`':
				res += font_size * 0.5f;
				break ;
			case 'I':
			case '[':
			case ']':
			case '"':
			case '*':
				res += font_size * 0.6f;
				break ;
			case 'l':
			case 't':
			case '(':
			case ')':
			case '<':
			case '>':
			case '{':
			case '}':
				res += font_size * 0.7f;
				break ;
			default:
				res += font_size;
				break ;
		}
	}
	return (res);
}

void Text::addText( int posX, int posY, int font_size, int color, std::string str, int limit )
{
	// std::cout << "writing " << str << " in R " << ((color >> 16) & 0xFF) << " G " << ((color >> 8) & 0xFF) << " B " << (color & 0xFF) << " A " << ((color >> 24) & 0xFF) << std::endl;
	// std::cout << "\tpos " << posX << ", " << posY << std::endl;
	int startX = posX;
	for (size_t i = 0, charLine = 0; i < str.size(); i++) {
		if (str[i] == '\n') {
			posY += 1.2f * font_size;
			posX = startX;
			charLine = 0;
		} else if (str[i] == ' ') {
			posX += font_size;
			++charLine;
		} else if (str[i] == '\t') {
			charLine += 4 - (charLine & 3);
			posX = startX + charLine * font_size;
		} else {
			if (posX - startX > limit) return ;
			char c = str[i];
			addQuads(c, posX, posY, font_size, font_size, color);
			if (c == 'i' || c == '.' || c == ':' || c == '!' || c == '\'' || c == ',' || c == ';' || c == '|' || c == '`') {
				posX += font_size * 0.5;
			} else if (c == 'I' || c == '[' || c == ']' || c == '"' || c == '*') {
				posX += font_size * 0.6;	
			} else if (c == 'l' || c == 't' || c == '(' || c == ')' || c == '<' || c == '>' || c == '{' || c == '}') {
				posX += font_size * 0.7;
			} else {
				posX += font_size;
			}
			++charLine;
		}
	}
}

void Text::toScreen( void )
{
	size_t tSize = _texts.size();
	if (tSize == 0) {
		return ;
	}

    glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, tSize * sizeof(GLint), &_texts[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(TEXT::SPECATTRIB);
	glVertexAttribIPointer(TEXT::SPECATTRIB, 1, GL_INT, 4 * sizeof(GLint), 0);
	glEnableVertexAttribArray(TEXT::COLATTRIB);
	glVertexAttribIPointer(TEXT::COLATTRIB, 1, GL_INT, 4 * sizeof(GLint), (void *)(1 * sizeof(GLint)));
	glEnableVertexAttribArray(TEXT::POSATTRIB);
	glVertexAttribIPointer(TEXT::POSATTRIB, 2, GL_INT, 4 * sizeof(GLint), (void *)(2 * sizeof(GLint)));

	check_glstate("Text::toScreen", false);

	glUseProgram(_shaderProgram);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, tSize / 4);
	glEnable(GL_DEPTH_TEST);

	_texts.clear();
}
