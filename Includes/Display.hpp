#ifndef DISPLAY_HPP
# define DISPLAY_HPP

# include "utils.hpp"
# define GOLDEN_RATIO 1.6180339887
# define WIN_HEIGHT 600
# define WIN_WIDTH WIN_HEIGHT * GOLDEN_RATIO
# define NUM_PARTS 10000
# define TICK 0.05

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _shaderProgram, _vao, _vbo;
		GLint _winWidth, _winHeight;
		int _fps, _tps;
		float _deltaTime;
		// Gui *_gui;

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void setup_array_buffer( void );

		void handleInputs( void );
		void render( void );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void start( void );
};

#endif
