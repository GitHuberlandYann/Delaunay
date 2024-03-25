#ifndef GUI_HPP
# define GUI_HPP

# include "Text.hpp"

namespace CONTAINER
{
	enum {
		TEXT,			// quite explicit
		VAR_INT,		// track and print var ptr to int
		VAR_FLOAT,		// track and print var ptr to float
		INPUT_TEXT,		// mod ptr to string with user's inputs
		BUTTON,			// calls ptr_function when pressed
		BOOL,			// mod ptr to bool to true/false
		SLIDER_INT,		// mod ptr to int linearly within given range
		SLIDER_FLOAT,	// mod ptr to float linearly within given range
		ENUM,			// (call fct | mod ptr to int) on each press
		COLOR, 			// mod RBBA ptrs in float range [0-1], but display them in int range [0-255]
	};
}

typedef struct s_container { // TODO simplify this to only use 2 void *, 2 int, 2 float
	int type = CONTAINER::TEXT;
	std::string name = "";
	void (*foo_ptr)( int ) = NULL;
	int *islider = NULL;
	int irange_start = 0, irange_end = 10;
	float *fslider = NULL;
	float frange_start = 0.0f, frange_end = 1.0f;
	int *enu = NULL, enu_index = 0;
	std::vector<std::string> enu_list = {};
	int selection = -1;
	std::array<float*, 4> color = {NULL, NULL, NULL, NULL};
	bool *bptr = NULL;
	std::string *input = NULL;
}				t_container;

const int title_height = 20;
const int font = 12;
const char ALPHABETA[127] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
const int min_win_width = 120;
const int min_win_height = 40;
typedef struct s_window {
	int id = 0;
	std::string title = "";
	std::array<int, 2> pos, size, offset = {0, 0};
	int selection = -1;
	std::vector<t_container> content = {};
}			t_window;

class Gui
{
	private:
		GLuint _shaderProgram;
		int _selection, _highlighted_window, _winWidth, _winHeight;
		unsigned _seed;
		bool _input_released, _moving_window, _resize_window, _moving_slider, _moving_color, _reading_inputs, _closing_window;
		std::vector<t_window> _content;
		std::vector<int> _draw_order;
		GLFWwindow *_window;
		GLFWcursor *_cursor;
		Text *_text;

		void setCursorPosWindow( t_window &win, int posX, int posY );
		int containerWidth( t_container &cont );
		void addContainer( t_container cont );
		void randomizeWindow( t_window &win );
		void putWindowOnTop( int index );
		void renderWindow( t_window &win, int windex );

	public:
		Gui( void );
		~Gui( void );

		void setWindowSize( int width, int height );
		int getHighlightedWindow( int previous );
		void setCursorPos( double posX, double posY );
		void setMouseButton( GLFWwindow *window, int button, int action );
		bool mouseControl( void );
		bool keyboardControl( void );

		void start( GLFWwindow *window );
		void render( void );

		void writeText( int posX, int posY, int font_size, int color, std::string str );
		bool createWindow( int id, std::string title, std::array<int, 2> pos = {20, 20}, std::array<int, 2> size = {250, title_height} );
		void resetWindow( int id, std::string title, std::string old_title = "", bool recursion = false );
		void renameWindow( int id, std::string title, std::string old_title = "" );
		void rmWindow( int id );
		void randomizeWindowAt( int id );
		void addText( std::string name );
		void addVarInt( std::string before, int *ptr, std::string after );
		void addVarFloat( std::string before, float *ptr, std::string after );
		void addInputText( std::string name, std::string *ptr, int limit = -1 );
		void addButton( std::string name, void (*foo_ptr)( int ), int *iptr = NULL, int i = -1 );
		void addBool( std::string name, bool *ptr );
		void addSliderInt( std::string name, int *ptr, int minRange = 0, int maxRange = 10 );
		void addSliderFloat( std::string name, float *ptr, float minRange = 0.0f, float maxRange = 1.0f );
		void addEnum( std::vector<std::string> enu_list, int *iptr, void (*foo_ptr)( int ) = NULL );
		void addColor( std::string name, std::array<float*, 4> color );
};

#endif
