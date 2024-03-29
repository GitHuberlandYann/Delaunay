#include "Display.hpp"

#if __linux__
# define IS_LINUX true
#else
# define IS_LINUX false
#endif

Display *display = NULL;
Gui *gui = NULL;

void set_display_callback( Display *dis, Gui *g )
{
	display = dis;
	gui = g;
}

static void window_size_callback( GLFWwindow *window, int width, int height )
{
	(void)window;
	// std::cout << "window resized to " << width << ", " << height << std::endl;
	if (display) {
		display->setWindowSize(width, height);
		if (IS_LINUX) {
			glViewport(0, 0, width, height);
		}
	}
}

void window_pos_callback( GLFWwindow *window, int posX, int posY )
{
	(void)window;
	// std::cout << "window pos set to " << posX << ", " << posY << std::endl;
	if (display) {
		(void)posX;(void)posY;
		// display->setWindowPos(posX, posY);
	}
}

// using this instead of window_size_callback because when you resize window from top border, window_pos_callback not called
void window_refresh_callback( GLFWwindow *window )
{
	// std::cout << "refresh callback" << std::endl;
	int posX, posY;
	glfwGetWindowPos(window, &posX, &posY);
	window_pos_callback(window, posX, posY);
	glfwGetWindowSize(window, &posX, &posY);
	window_size_callback(window, posX, posY);
}

void cursor_pos_callback( GLFWwindow *window, double posX, double posY )
{
	(void)window;
	if (gui) {
		gui->setCursorPos(posX, posY);
	}
}

void mouse_button_callback( GLFWwindow *window, int button, int action, int mods )
{
	(void)mods;
	if (gui) {
		gui->setMouseButton(window, button, action);
	}
}

void gui_randomize_callback( int index )
{
	if (gui) {
		gui->randomizeWindowAt(index);
	}
}

void boid_settings_callback( int index )
{
	(void)index;
	if (display) {
		display->boid_settings();
	}
}

void debug_window_callback( int index )
{
	(void)index;
	if (display) {
		display->debug_window();
	}
}

void error_callback( int error, const char *msg )
{
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    std::cerr << s << std::endl;
}

// ************************************************************************** //
//                                Inputs                                      //
// ************************************************************************** //

namespace INPUT
{
	std::string *message = NULL;
	int cursor = 0, limit = -1;

	void character_callback( GLFWwindow* window, unsigned int codepoint )
	{
		(void)window;
		if (!message) return ;
		if (limit != -1 && static_cast<int>(message->size()) >= limit) return ;

		if (codepoint < 32 || codepoint > 126) {
			std::cout << __func__ << ": codepoint out of range: " << codepoint << std::endl;
			return ;
		}
		// std::cout << "codepoint you just pressed: " << codepoint << " => " << ALPHABETA[codepoint - 32] << std::endl;
		if (cursor == static_cast<int>(message->size())) {
			*message += ALPHABETA[codepoint - 32];
		} else {
			*message = message->substr(0, cursor) + ALPHABETA[codepoint - 32] + message->substr(cursor);
		}
		++cursor;
	}

	void moveCursor( bool right, bool control )
	{
		if (!message) return ;
		cursor += (right) ? 1 : -1;
		if (cursor > static_cast<int>(message->size())) {
			cursor = message->size();
		} else if (cursor < 0) {
			cursor = 0;
		} else if (control && (*message)[cursor] != ' ') {
			moveCursor(right, control);
		}
	}

	void rmLetter( void )
	{
		if (!cursor || !message) return ;
		*message = message->substr(0, cursor - 1) + message->substr(cursor);
		--cursor;
	}

	void setMessagePtr( std::string *ptr, int lim )
	{
		message = ptr;
		cursor = (message) ? message->size() : 0;
		limit = lim;
	}

	std::string *getMessagePtr( void )
	{
		return (message);
	}

	int getCursor( void )
	{
		return (cursor);
	}
}
