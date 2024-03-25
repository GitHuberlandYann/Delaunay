#ifndef DISPLAY_HPP
# define DISPLAY_HPP

# include "utils.hpp"
# define GOLDEN_RATIO 1.6180339887
# define WIN_HEIGHT 600
# define WIN_WIDTH WIN_HEIGHT * GOLDEN_RATIO
# define NUM_PARTS 10000
# define TICK 0.05

# include "delaunay.hpp"
# include "Gui.hpp"

typedef struct s_shaderInput {
	Vertex v;
	float radius;
}				t_shaderInput;

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _shaderProgram, _vao, _vbo;
		GLint _uniMaxRadius, _uniBigColor, _uniSmallColor;
		GLint _winWidth, _winHeight;
		int _fps, _tps, _nb_points;
		bool _input_released;
		float _deltaTime;
		unsigned _seed;
		std::array<float, 4> _bigCol, _smallCol;
		std::vector<Vertex> _points;
		std::vector<Triangle> _delaunay;
		std::vector<t_shaderInput> _vertices;
		Gui *_gui;

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void setup_array_buffer( void );
		void setup_delaunay( void );

		void handleInputs( void );
		void render( void );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void start( void );
};

#endif
