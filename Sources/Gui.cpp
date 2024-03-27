#include "Gui.hpp"
#include "callbacks.hpp"
#include "random.hpp"

Gui::Gui( void ) : _selection(-1), _highlighted_window(-1), _input_released(true),
	_moving_window(false), _resize_window(false), _moving_slider(false), _moving_color(false),
	_reading_inputs(false), _closing_window(false), _cursor(NULL)
{
	_text = new Text();
}

Gui::~Gui( void )
{
	delete _text;
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

// posX, posY are relative to t_window's position
void Gui::setCursorPosWindow( t_window &win, int posX, int posY )
{
	if (inRectangle(posX, posY, 0, 0, win.size[0], title_height)) { // title bar
		if (inRectangle(posX, posY, win.size[0] - title_height, 0, title_height, title_height)) {
			win.selection = 0; // close window
		} else {
			win.offset = {posX, posY};
			win.selection = 1; // move window
		}
	} else if (inRectangle(posX, posY, win.size[0] - title_height / 2, win.size[1] - title_height / 2, title_height / 2, title_height / 2)) {
		win.selection = 2; // resize window
	} else { // containers
		win.selection = -1;
		int index = 3;
		for (auto &cont : win.content) {
			if (title_height * 1.5f * (index - 2) + title_height > win.size[1] - 10) return ;
			switch (cont.type) {
				case CONTAINER::SLIDER_INT:
				case CONTAINER::SLIDER_FLOAT:
				case CONTAINER::INPUT_TEXT:
					if (inRectangle(posX, posY, 10, title_height * 1.5f * (index - 2), win.size[0] / 2, title_height)) {
						win.selection = index;
						return ;
					}
					break ;
				case CONTAINER::BUTTON:
				case CONTAINER::ENUM:
					if (inRectangle(posX, posY, 10, title_height * 1.5f * (index - 2), win.size[0] - 20, title_height)) {
						win.selection = index;
						return ;
					}
					break ;
				case CONTAINER::BOOL:
					if (inRectangle(posX, posY, 10, title_height * 1.5f * (index - 2), title_height, title_height)) {
						win.selection = index;
						return ;
					}
					break ;
				case CONTAINER::COLOR:
					int div = (cont.color[3]) ? 4 : 3, width = (win.size[0] / 2 - ((div == 4) ? 30 : 20)) / div;
					if (inRectangle(posX, posY, 10, title_height * 1.5f * (index - 2), width, title_height)) {
						win.selection = index;
						cont.selection = 0;
						win.offset[0] = posX + win.pos[0];
						return ;
					} else if (inRectangle(posX, posY, 20 + width, title_height * 1.5f * (index - 2), width, title_height)) {
						win.selection = index;
						cont.selection = 1;
						win.offset[0] = posX + win.pos[0];
						return ;
					} else if (inRectangle(posX, posY, 30 + 2 * width, title_height * 1.5f * (index - 2), width, title_height)) {
						win.selection = index;
						cont.selection = 2;
						win.offset[0] = posX + win.pos[0];
						return ;
					} else if (div == 4 && inRectangle(posX, posY, 40 + 3 * width, title_height * 1.5f * (index - 2), width, title_height)) {
						win.selection = index;
						cont.selection = 3;
						win.offset[0] = posX + win.pos[0];
						return ;
					}
					break ;
			}
			++index;
		}
	}
}

int Gui::containerWidth( t_container &cont )
{
	switch (cont.type) {
		case CONTAINER::TEXT:
		case CONTAINER::BUTTON:
			return (30 + _text->textWidth(font, cont.name));
		case CONTAINER::VAR_INT:
		case CONTAINER::VAR_FLOAT:
			return (30 + _text->textWidth(font, cont.enu_list[0] + cont.enu_list[1]) + 6 * font);
		case CONTAINER::BOOL:
			return (30 + title_height + _text->textWidth(font, cont.name));
		case CONTAINER::INPUT_TEXT:
			return (2 * std::max(20 + cont.irange_start * font, 30 + _text->textWidth(font, cont.name)));
		case CONTAINER::SLIDER_INT:
		case CONTAINER::SLIDER_FLOAT:
			return (2 * (30 + _text->textWidth(font, cont.name)));
		case CONTAINER::COLOR:
			return (2 * (30 + title_height + _text->textWidth(font, cont.name)));
	}
	return (0);
}

void Gui::addContainer( t_container cont )
{
	t_window &win = _content.back();

	win.content.push_back(cont);
	int width = containerWidth(win.content.back());
	if (win.size[0] < width) {
		win.size[0] = width;
	}
	int height = title_height * 1.5f * (win.content.size() + 1);
	if (win.size[1] < height) {
		win.size[1] = height;
	}
}

void Gui::randomizeWindow( t_window &win )
{
	for (auto &cont : win.content) {
		switch (cont.type) {
			case CONTAINER::VAR_INT:
			case CONTAINER::SLIDER_INT:
				*cont.islider = cont.irange_start + Random::randomFloat(_seed) * (cont.irange_end - cont.irange_start);
				break ;
			case CONTAINER::VAR_FLOAT:
			case CONTAINER::SLIDER_FLOAT:
				*cont.fslider = cont.frange_start + Random::randomFloat(_seed) * (cont.frange_end - cont.frange_start);
				break ;
			case CONTAINER::ENUM:
				cont.enu_index = Random::randomFloat(_seed) * (cont.enu_list.size());
				if (cont.enu) *cont.enu = cont.enu_index;
				break ;
			case CONTAINER::COLOR:
				*cont.color[0] = Random::randomFloat(_seed);
				*cont.color[1] = Random::randomFloat(_seed);
				*cont.color[2] = Random::randomFloat(_seed);
				if (cont.color[3]) *cont.color[3] = Random::randomFloat(_seed);
				break ;
		}
	}
}

void Gui::putWindowOnTop( int index )
{
	size_t i = 0, doSize = _draw_order.size();
	if (!doSize) return ;
	for (; _draw_order[i] != index && i < doSize; ++i);
	if (i == doSize || i == doSize - 1) return ;
	for (; i < doSize - 1; ++i) {
		_draw_order[i] = _draw_order[i + 1];
	}
	_draw_order[i] = index;
}

void Gui::renderWindow( t_window &win, int windex )
{
	// std::cout << "rendering gui window " << win.title << std::endl;
	_text->addQuads(0, win.pos[0], win.pos[1] + title_height, win.size[0], win.size[1] - title_height, RGBA::BACK_WINDOW);
	_text->addQuads(0, win.pos[0], win.pos[1], win.size[0] - title_height, title_height, (windex == _highlighted_window) ? RGBA::TITLE_SELECTED_WINDOW : RGBA::TITLE_WINDOW); // title bar
	_text->addQuads(1, win.pos[0] + win.size[0] - title_height, win.pos[1], title_height, title_height, (win.selection == 0) ? RGBA::CLOSE_WINDOW_HOVER : RGBA::CLOSE_WINDOW);
	_text->addText(win.pos[0] + 10, win.pos[1] + 4, font, RGBA::WHITE, win.title);
	_text->addTriangle(0, {win.pos[0] + win.size[0], win.pos[1] + win.size[1] - title_height}, {win.pos[0] + win.size[0], win.pos[1] + win.size[1]},
			{win.pos[0] + win.size[0] - title_height, win.pos[1] + win.size[1]}, RGBA::MOVE_WINDOW);
	
	int index = 3;
	std::string str;
	for (auto &cont : win.content) {
		if (title_height * 1.5f * (index - 2) + title_height > win.size[1] - 10) return ;
		int posY = win.pos[1] + title_height * 1.5f * (index - 2);
		switch (cont.type) {
			case CONTAINER::TEXT:
				_text->addText(win.pos[0] + 10, posY + 4, font, RGBA::WHITE, cont.name, win.size[0] - 30);
				break ;
			case CONTAINER::VAR_INT:
				str = cont.enu_list[0] + std::to_string(*cont.islider) + cont.enu_list[1];
				_text->addText(win.pos[0] + 10, posY + 4, font, RGBA::WHITE, str, win.size[0] - 30);
				break ;
			case CONTAINER::VAR_FLOAT:
				str = cont.enu_list[0] + to_string_with_precision(*cont.fslider, 4) + cont.enu_list[1];
				_text->addText(win.pos[0] + 10, posY + 4, font, RGBA::WHITE, str, win.size[0] - 30);
				break ;
			case CONTAINER::INPUT_TEXT:
				_text->addQuads(0, win.pos[0] + 10, posY, win.size[0] / 2, title_height, RGBA::MOVE_WINDOW);
				_text->addText(win.pos[0] + 20, posY + 4, font, RGBA::WHITE, cont.input->c_str());
				_text->addText(win.pos[0] + 20 + win.size[0] / 2, posY + 4, font, RGBA::WHITE, cont.name,  win.size[0] / 2 - 30);
				if (cont.input == INPUT::getMessagePtr()) {
					_text->addQuads(0, win.pos[0] + 20 + _text->textWidth(font, *cont.input, INPUT::getCursor()), posY, font / 4, title_height, RGBA::CLOSE_WINDOW);
				}
				break ;
			case CONTAINER::BUTTON:
				_text->addQuads(0, win.pos[0] + 10, posY, win.size[0] - 20, title_height, (win.selection == index) ? RGBA::SLIDER_HOVER : RGBA::BUTTON);
				str = cont.name;
				_text->addText(win.pos[0] + (win.size[0] - _text->textWidth(font, str)) / 2, posY + 4, font, RGBA::WHITE, str);
				break ;
			case CONTAINER::BOOL:
				_text->addQuads(*cont.bptr, win.pos[0] + 10, posY, title_height, title_height, RGBA::BUTTON);
				_text->addText(win.pos[0] + 20 + title_height, posY + 4, font, RGBA::WHITE, cont.name, win.size[0] - 40 - title_height);
				break ;
			case CONTAINER::SLIDER_INT:
				_text->addQuads(0, win.pos[0] + 10, posY, win.size[0] / 2, title_height, RGBA::BUTTON);
				_text->addQuads(0, win.pos[0] + 10 + (win.size[0] / 2 - title_height + 8) * getPercent(*cont.islider, cont.irange_start, cont.irange_end),
						posY + 2, title_height - 8, title_height - 4, (win.selection == index) ? RGBA::SLIDER_HOVER : RGBA::SLIDER);
				str = std::to_string(*cont.islider);
				_text->addText(win.pos[0] + 10 + win.size[0] / 4 - _text->textWidth(font, str) / 2, posY + 4, font, RGBA::WHITE, str);
				_text->addText(win.pos[0] + win.size[0] / 2 + 20, posY + 4, font, RGBA::WHITE, cont.name, win.size[0] / 2 - 30);
				break ;
			case CONTAINER::SLIDER_FLOAT:
				_text->addQuads(0, win.pos[0] + 10, posY, win.size[0] / 2, title_height, RGBA::BUTTON);
				_text->addQuads(0, win.pos[0] + 10 + (win.size[0] / 2 - title_height + 8) * getPercent(*cont.fslider, cont.frange_start, cont.frange_end),
						posY + 2, title_height - 8, title_height - 4, (win.selection == index) ? RGBA::SLIDER_HOVER : RGBA::SLIDER);
				str = to_string_with_precision(*cont.fslider, cont.irange_start, false);
				_text->addText(win.pos[0] + 10 + win.size[0] / 4 - _text->textWidth(font, str) / 2, posY + 4, font, RGBA::WHITE, str);
				_text->addText(win.pos[0] + win.size[0] / 2 + 20, posY + 4, font, RGBA::WHITE, cont.name, win.size[0] / 2 - 30);
				break ;
			case CONTAINER::ENUM:
				_text->addQuads(0, win.pos[0] + 10, posY, win.size[0] - 20, title_height, (win.selection == index) ? RGBA::SLIDER_HOVER : RGBA::BUTTON);
				str = cont.enu_list[cont.enu_index];
				_text->addText(win.pos[0] + (win.size[0] - _text->textWidth(font, str)) / 2, posY + 4, font, RGBA::WHITE, str);
				break ;
			case CONTAINER::COLOR:
				int div = (cont.color[3]) ? 4 : 3, width = (win.size[0] / 2 - ((div == 4) ? 30 : 20)) / div;
				_text->addQuads(0, win.pos[0] + 10, posY, width, title_height, RGBA::BUTTON);
				str = std::to_string(static_cast<int>(*cont.color[0] * 255));
				_text->addText(win.pos[0] + 10 + (width - _text->textWidth(font, str)) / 2, posY + 4, font, RGBA::WHITE, str);
				_text->addQuads(0, win.pos[0] + 20 + width, posY, width, title_height, RGBA::BUTTON);
				str = std::to_string(static_cast<int>(*cont.color[1] * 255));
				_text->addText(win.pos[0] + 20 + width + (width - _text->textWidth(font, str)) / 2, posY + 4, font, RGBA::WHITE, str);
				_text->addQuads(0, win.pos[0] + 30 + 2 * width, posY, width, title_height, RGBA::BUTTON);
				str = std::to_string(static_cast<int>(*cont.color[2] * 255));
				_text->addText(win.pos[0] + 30 + 2 * width + (width - _text->textWidth(font, str)) / 2, posY + 4, font, RGBA::WHITE, str);
				if (div == 4) {
					_text->addQuads(0, win.pos[0] + 40 + 3 * width, posY, width, title_height, RGBA::BUTTON);
					str = std::to_string(static_cast<int>(*cont.color[3] * 255));
					_text->addText(win.pos[0] + 40 + 3 * width + (width - _text->textWidth(font, str)) / 2, posY + 4, font, RGBA::WHITE, str);
				}
				_text->addQuads(0, win.pos[0] + win.size[0] / 2 + 20, posY, title_height, title_height, rgbaFromVec({*cont.color[0], *cont.color[1], *cont.color[2], (div == 4) ? *cont.color[3] : 1.0f}));
				_text->addText(win.pos[0] + win.size[0] / 2 + 30 + title_height, posY + 4, font, RGBA::WHITE, cont.name, win.size[0] / 2 - 40 - title_height);
				break ;
		}
		++index;
	}
	// _text->addText(win.pos[0] + 10, win.pos[1] + title_height * 1.5f * (index - 2), 12, RGBA::WHITE, "selection " + std::to_string(win.selection));
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Gui::setWindowSize( int width, int height )
{
	for (auto &win : _content) {
		win.pos[0] *= static_cast<float>(width) / _winWidth;
		win.pos[1] *= static_cast<float>(height) / _winHeight;
	}
	_winWidth = width;
	_winHeight = height;
	_text->setWindowSize(width, height);
}

// return highlighted window's id, or previous if no highlighted window
int Gui::getHighlightedWindow( int previous )
{
	if (_highlighted_window == -1) {
		return (previous);
	}
	int res = _content[_highlighted_window].id;
	if (res < 0) {
		return (previous);
	}
	return (res);
}

void Gui::setCursorPos( double posX, double posY )
{
	if (_moving_window) {
		t_window &win = _content[_selection];
		win.pos[0] = posX - win.offset[0];
		win.pos[1] = posY - win.offset[1];
		if (win.pos[0] < 0) win.pos[0] = 0;
		else if (win.pos[0] > _winWidth - title_height) win.pos[0] = _winWidth - title_height;
		if (win.pos[1] < 0) win.pos[1] = 0;
		else if (win.pos[1] > _winHeight - title_height) win.pos[1] = _winHeight - title_height;
		return ;
	} else if (_resize_window) {
		t_window &win = _content[_selection];
		win.size[0] = posX - win.pos[0];
		win.size[1] = posY - win.pos[1];
		if (win.size[0] < min_win_width) win.size[0] = min_win_width;
		if (win.size[1] < min_win_height) win.size[1] = min_win_height;
		return ;
	} else if (_moving_slider) {
		t_window &win = _content[_selection];
		t_container &cont = win.content[win.selection - 3];
		if (cont.type == CONTAINER::SLIDER_INT) {
			*cont.islider = static_cast<int>(gradient(posX, win.pos[0] + 10, win.pos[0] + 10 + win.size[0] / 2, cont.irange_start, cont.irange_end));
		} else {
			*cont.fslider = gradient(posX, win.pos[0] + 10, win.pos[0] + 10 + win.size[0] / 2, cont.frange_start, cont.frange_end);
		}
		return ;
	} else if (_moving_color) {
		t_window &win = _content[_selection];
		int diff = posX - win.offset[0];
		win.offset[0] = posX;
		t_container &cont = win.content[win.selection - 3];
		*cont.color[cont.selection] += diff / 200.0f;
		if (*cont.color[cont.selection] < 0) *cont.color[cont.selection] = 0;
		if (*cont.color[cont.selection] > 1.0f) *cont.color[cont.selection] = 1.0f;
		return ;
	}

	for (int i = _draw_order.size() - 1; i >= 0; --i) {
		t_window &win = _content[_draw_order[i]];
		if (inRectangle(posX, posY, win.pos[0], win.pos[1], win.size[0], win.size[1])) {
			setCursorPosWindow(win, posX - win.pos[0], posY - win.pos[1]);
			_selection = _draw_order[i];
			return ;
		}
	}
	_selection = -1;
}

void Gui::setMouseButton( GLFWwindow *window, int button, int action )
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (_reading_inputs && action == GLFW_PRESS) { // no matter where you click, we reset reading inputs
			INPUT::setMessagePtr(NULL, -1);
			_reading_inputs = false;
		}
		if (action == GLFW_PRESS && _selection != -1) {
			_highlighted_window = _selection;
			putWindowOnTop(_selection);
			t_window &win = _content[_selection];
			switch (win.selection) {
				case -1: // no select
					break ;
				case 0: // close window
					rmWindow(win.id);
					_selection = -1;
					_closing_window = true;
					break ;
				case 1: // move around
					_moving_window = true;
					break ;
				case 2: // resize window
					_resize_window = true;
					_cursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
					glfwSetCursor(window, _cursor);
					break ;
				default: // on container
					t_container &cont = win.content[win.selection - 3];
					switch (cont.type) {
						case CONTAINER::INPUT_TEXT:
							INPUT::setMessagePtr(cont.input, cont.irange_start);
							_reading_inputs = true;
							break ;
						case CONTAINER::SLIDER_INT:
						case CONTAINER::SLIDER_FLOAT:
							_moving_slider = true;
							break ;
						case CONTAINER::COLOR:
							_moving_color = true;
							break ;
						case CONTAINER::BUTTON:
							cont.foo_ptr((cont.islider) ? *cont.islider : cont.irange_start);
							break ;
						case CONTAINER::BOOL:
							*cont.bptr = !*cont.bptr;
							break ;
						case CONTAINER::ENUM:
							++cont.enu_index;
							if (cont.enu_index >= static_cast<int>(cont.enu_list.size())) {
								cont.enu_index = 0;
							}
							if (cont.enu) *cont.enu = cont.enu_index;
							if (cont.foo_ptr) cont.foo_ptr(cont.enu_index);
							break ;
					}
					break ;
			}
		} else if (action == GLFW_RELEASE) {
			if (_resize_window) glfwDestroyCursor(_cursor);
			_moving_window = false;
			_resize_window = false;
			_moving_slider = false;
			_moving_color = false;
			_closing_window = false;
		}
	}
}

bool Gui::mouseControl( void )
{
	return (_selection != -1 || _closing_window);
}

bool Gui::keyboardControl( void )
{
	return (_reading_inputs);
}

void Gui::start( GLFWwindow *window )
{
	_window = window;
	_shaderProgram = _text->start();
	check_glstate("Gui successfully started", true);
}

void Gui::render( void )
{
	// _text->addText(10, 10, 20, RGBA::WHITE, "window " + std::to_string(_highlighted_window));

	if (_reading_inputs) { // window needed here
		if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE
			&& glfwGetKey(_window, GLFW_KEY_LEFT) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
			_input_released = true;
		} else if (_input_released) {
			_input_released = false;
			if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				INPUT::setMessagePtr(NULL, -1);
				_reading_inputs = false;
			} else if (glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
				INPUT::rmLetter();
			} else if (glfwGetKey(_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
				INPUT::moveCursor(false, glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
			} else if (glfwGetKey(_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
				INPUT::moveCursor(true, glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
			} 
		}
	}

	for (int index : _draw_order) {
		renderWindow(_content[index], index);
	}

	_text->toScreen();
}

void Gui::writeText( int posX, int posY, int font_size, int color, std::string str )
{
	_text->addText(posX, posY, font_size, color, str);
}

//
// > sub windows <
//

/**
 * @brief create new Gui window to be displayed when Gui::render() is called
 * @param id value returned by Gui::getHighlighted if this window is highlighted
 * @param title string displayed on top of window
 * @param pos ivec2 representing position of top left corner of window on GLFWwindow
 * @param size ivec2 representing width and hight of created window
 * @return true if new window created, false if window with same id and title already exists, in which case said window has beed deleted
 */
bool Gui::createWindow( int id, std::string title, std::array<int, 2> pos, std::array<int, 2> size )
{
	int index;
	bool posOk = false;
	while (!posOk) {
		posOk = true;
		index = 0;
		for (auto &win : _content) {
			if (win.id == id && win.title == title) {
				_highlighted_window = index;
				putWindowOnTop(index);
				rmWindow(id);
				return (false);
			}
			if (win.pos[1] == pos[1]) {
				pos[0] += title_height;
				pos[1] += title_height;
				posOk = false;
			}
			++index;
		}
		if (posOk) {
			posOk = false;
			if (pos[0] < 0) pos[0] = 0;
			else if (pos[0] > _winWidth - title_height) pos[0] = _winWidth - title_height;
			else if (pos[1] < 0) pos[1] = 0;
			else if (pos[1] > _winHeight - title_height) pos[1] = _winHeight - title_height;
			else posOk = true;
		}
	}
	_highlighted_window = _content.size();
	_draw_order.push_back(_highlighted_window);
	_content.push_back({id, title, pos, size});
	return (true);
}

/**
 * @brief Find windows with matching id [and title] and reset their content, if no window found, create a new window
 * @param id id of targeted window
 * @param title replace window's title with this one
 * @param old_title optional. If given, only del window with this id and title
 */
void Gui::resetWindow( int id, std::string title, std::string old_title, bool recursion )
{
	std::array<int, 2> pos = {20, 20};
	int index = 0;
	bool winDestroyed = false;
	for (auto &win : _content) {
		if (win.id == id && (!old_title[0] || win.title == old_title)) {
			_highlighted_window = index;
			pos = win.pos;
			putWindowOnTop(index);
			rmWindow(id);
			winDestroyed = true;
			break ;
		}
		++index;
	}
	if (winDestroyed) {
		resetWindow(id, title, old_title, true); // recursively destroy all window with given id
	}
	if (recursion) {
		return ;
	}
	createWindow(id, title, pos);
}

/**
 * @brief Find window with matching id and title and change it's title
 * @param id id of targeted window
 * @param title replace window's title with this one
 * @param old_title optional. title of targeted window
 */
void Gui::renameWindow( int id, std::string title, std::string old_title )
{
	for (auto &win : _content) {
		if (win.id == id) {
			if (old_title[0] && win.title != old_title) continue ;
			win.title = title;
			return ;
		}
	}
}

/**
 * @brief Remove window with id 'id'
 * @param id id of windows to be deleted
 */
void Gui::rmWindow( int id )
{
	int index = 0;
	for (auto &win : _content) {
		if (win.id == id) {
			break ;
		}
		++index;
	}
	if (index >= static_cast<int>(_content.size())) return ;
	_content.erase(_content.begin() + index);
	for (int &d : _draw_order) {
		if (d > index) --d;
	}
	_draw_order.pop_back();
	_highlighted_window = -1;
}

/**
 * @brief Randomize content of window with id 'id'
 * @param id id of windows to be randomized
 */
void Gui::randomizeWindowAt( int id )
{
	for (auto &win : _content) {
		if (win.id == id) {
			randomizeWindow(win);
		}
	}
}

/**
 * @brief add Text container to the last created window with Gui::createWindow
 * @param name string displayed in container
 */
void Gui::addText( std::string name )
{
	if (_content.empty()) return ;

	addContainer({CONTAINER::TEXT, name});
}

/**
 * @brief add VarInt container to the last created window with Gui::createWindow
 * @param ptr pointer to the integer to be displayed
 * @param before string displayed before nbr
 * @param after string displayed after nbr
 */
void Gui::addVarInt( std::string before, int *ptr, std::string after )
{
	if (_content.empty() || !ptr) return ;

	addContainer({CONTAINER::VAR_INT, "", NULL, ptr, 0, 0, NULL, 0, 0, NULL, 0, {before, after}});
}

/**
 * @brief add VarFloat container to the last created window with Gui::createWindow
 * @param ptr pointer to the float to be displayed
 * @param before string displayed before nbr
 * @param after string displayed after nbr
 */
void Gui::addVarFloat( std::string before, float *ptr, std::string after )
{
	if (_content.empty() || !ptr) return ;

	addContainer({CONTAINER::VAR_FLOAT, "", NULL, NULL, 0, 0, ptr, 0, 0, NULL, 0, {before, after}});
}

/**
 * @brief add InputText container to the last created window with Gui::createWindow
 * @param ptr pointer to the string to be modified by user
 * @param size optional. limits size of input
 */
void Gui::addInputText( std::string name, std::string *ptr, int limit )
{
	if (_content.empty() || !ptr || (limit != -1 && limit < static_cast<int>(ptr->size()))) return ;

	addContainer({CONTAINER::INPUT_TEXT, name, NULL, NULL, limit, 0, NULL, 0, 0, NULL, 0, {}, 0, {NULL, NULL, NULL, NULL}, NULL, ptr});
}

/**
 * @brief add Button container to the last created window with Gui::createWindow
 * @param name string displayed onto button
 * @param foo_ptr function called by each press on the button
 * @param iptr optional. if given, ptr to int argument of foo_ptr
 */
void Gui::addButton( std::string name, void (*foo_ptr)( int ), int *iptr, int i )
{
	if (_content.empty() || !foo_ptr) return ;

	addContainer({CONTAINER::BUTTON, name, foo_ptr, iptr, i});
}

/**
 * @brief add Bool container to the last created window with Gui::createWindow
 * @param name string displayed square
 * @param ptr pointer to bool
 */
void Gui::addBool( std::string name, bool *ptr )
{
	if (_content.empty() || !ptr) return ;

	addContainer({CONTAINER::BOOL, name, NULL, NULL, 0, 0, NULL, 0, 0, NULL, 0 , {}, 0, {NULL, NULL, NULL, NULL}, ptr});
}

/**
 * @brief add SliderInt container to the last created window with Gui::createWindow
 * @param name string displayed after slider
 * @param ptr pointer to int variable modified by slider
 * @param minRange the minimum value the variable can be set to
 * @param minRange the maximum value the variable can be set to
 */
void Gui::addSliderInt( std::string name, int *ptr, int minRange, int maxRange )
{
	if (_content.empty() || !ptr) return ;

	addContainer({CONTAINER::SLIDER_INT, name, NULL, ptr, minRange, maxRange});
}

/**
 * @brief add SliderFloat container to the last created window with Gui::createWindow
 * @param name string displayed after slider
 * @param ptr pointer to float variable modified by slider
 * @param minRange the minimum value the variable can be set to
 * @param minRange the maximum value the variable can be set to
 * @param precision optional. nb decimals displayed after point
 */
void Gui::addSliderFloat( std::string name, float *ptr, float minRange, float maxRange, int precision )
{
	if (_content.empty() || !ptr) return ;

	addContainer({CONTAINER::SLIDER_FLOAT, name, NULL, NULL, precision, 0, ptr, minRange, maxRange});
}

/**
 * @brief add Enum container to the last created window with Gui::createWindow
 * @param enu_list vector of string displayed on button
 * @param iptr pointer to int variable modified by button, or NULL
 * @param foo_ptr function called by each press on the button, or NULL
 */
void Gui::addEnum( std::vector<std::string> enu_list, int *iptr, void (*foo_ptr)( int ) )
{
	if (_content.empty() || enu_list.size() < 2 || (!iptr && !foo_ptr)) return ;

	addContainer({CONTAINER::ENUM, "", foo_ptr, NULL, 0, 0, NULL, 0, 0, iptr, (iptr) ? *iptr : 0, enu_list});
}

/**
 * @brief add Color container to the last created window with Gui::createWindow
 * @param name string displayed after color's RGBA channels
 * @param color vec4 of pointers to float in range[0:1], color[3] (=alpha) is optional
 */
void Gui::addColor( std::string name, std::array<float*, 4> color )
{
	if (_content.empty() || (!color[0] && !color[1] && !color[2])) return ;

	addContainer({CONTAINER::COLOR, name, NULL, NULL, 0, 0, NULL, 0, 0, NULL, 0, {}, -1, color});
}
