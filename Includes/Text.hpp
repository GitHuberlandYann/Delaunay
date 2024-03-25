#ifndef TEXT_HPP
# define TEXT_HPP

# include "utils.hpp"

# include <vector>

namespace RGBA
{
	const int WHITE = 0xFFFFFFFF;
	const int BLACK = 0xFF000000;
	const int BACK_WINDOW = 0xDF202020;
	const int TITLE_WINDOW = 0xDF294A7A;
	const int TITLE_SELECTED_WINDOW = 0xDF597AAA;
	const int CLOSE_WINDOW = 0xDFDD0000;
	const int CLOSE_WINDOW_HOVER = 0xFFFF0000;
	const int BUTTON = 0xDF274972;
	const int SLIDER = 0xDF3D85E0;
	const int SLIDER_HOVER = 0xDF4D95F0;
	const int MOVE_WINDOW = 0xDF1E3044;
}

namespace TEXT
{
	enum {
		SPECATTRIB,
		COLATTRIB,
		POSATTRIB
	};
}

class Text
{
	private:
        GLuint _vao, _vbo, _shaderProgram;
		GLint _uniWidth, _uniHeight;
		GLuint _texture;
		std::vector<int> _texts;

        void setup_shader( void );
		void setup_communication_shader( void );
		void load_texture( void );

	public:
		Text( void );
		~Text( void );

		GLuint start( void );
		void setWindowSize( int width, int height );
		void addTriangle( int spec, std::array<int, 2> a, std::array<int, 2> b, std::array<int, 2> c, int color );
		void addQuads( int spec, int posX, int posY, int width, int height, int color );
		int textWidth( int font_size, std::string str, int limit = -1 );
        void addText( int posX, int posY, int font_size, int color, std::string str, int limit = 0x7FFFFFFF );
		void toScreen( void );
};

#endif
