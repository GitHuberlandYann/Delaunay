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

namespace BUFFER {
	const int POINTS = 0;
	const int TRIANGLES = 1;
};

typedef struct s_updateShaderInput {
	Vertex v = {0.0f, 0.0f};
	Vertex speed = {0.0f, 0.0f};
}				t_updateShaderInput;

typedef struct s_renderShaderInput {
	Vertex v;
	float radius;
}				t_renderShaderInput;

typedef struct s_boidSettings {
	float visualRange = 75.0f;
	float centeringFactor = 0.005f; // 0.005 used in coherence
	float minDistance = 20.0f; // 20 used in separation, dist to stay away from other boids
	float avoidFactor = 0.05f; // 0.05 used in separation
	float matchingFactor = 0.05f; // 0.05 used in alignment
	float length = 15.0f;
	float width = 2.7f;
	float speedLimit = 100.0f;
	float turnFactor = 1.0f; // used in keep within bounds
}				t_boidSettings;

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _pointsUpdateProgram, _pointsRenderProgram, _boidsRenderProgram, _shaderProgram, _vaos[2], _vbos[2];
		GLint _uniPZoom, _uniPCenter, _uniZoom, _uniCenter, _uniDeltaT, _uniMaxRadius, _uniBigColor, _uniSmallColor;
		GLint _uniBZoom, _uniBCenter, _uniBBoidLength, _uniBBoidWidth;
		GLint _winWidth, _winHeight;
		t_boidSettings _boidSettings;
		int _fps, _tps, _nb_points;
		bool _input_released, _update_boids, _update_points, _draw_points, _draw_boids, _draw_delaunay;
		float _deltaTime, _speed_multiplier, _zoom;
		std::array<float, 2> _center;
		unsigned _seed;
		std::array<float, 4> _bigCol, _smallCol;
		std::vector<Vertex> _points;
		std::vector<Triangle> _delaunay;
		std::vector<t_updateShaderInput> _update_vertices;
		std::vector<t_renderShaderInput> _vertices;
		Gui *_gui;

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void setup_array_buffer( int buffer );
		void setup_delaunay( void );
		void reset_delaunay( void );

		void handleInputs( void );
		void render( void );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void setWindowSize( int width, int height );
		void debug_window( void );
		void boid_settings( void );

		void start( void );
};

#endif
