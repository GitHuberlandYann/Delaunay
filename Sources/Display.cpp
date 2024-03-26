#include "Display.hpp"
#include "random.hpp"
#include "callbacks.hpp"

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT), _nb_points(1000),
	_update_points(false), _draw_points(true), _draw_delaunay(false), _speed_multiplier(1.0f),
	_zoom(1.0f), _center({0.0f, 0.0f}), _seed(1503),
	_bigCol({0.8f, 0.8f, 0.8f, 1.0f}), _smallCol({0.2f, 0.2f, 0.2f, 1.0f})
{
	_gui = new Gui();
}

Display::~Display( void )
{
	std::cout << "Destructor of display called" << std::endl;

	glDeleteProgram(_pointsUpdateProgram);
	glDeleteProgram(_pointsRenderProgram);
	glDeleteProgram(_shaderProgram);

	glDeleteBuffers(2, _vbos);
	glDeleteVertexArrays(2, _vaos);

	glfwMakeContextCurrent(NULL);
    glfwTerminate();

	delete _gui;

	check_glstate("Display successfully destructed", true);
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

void Display::setup_window( void )
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
    	std::cerr << "glfwInit failure" << std::endl;
        exit(1);
    }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	// glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);

	_window = glfwCreateWindow(_winWidth, _winHeight, "Delaunay", nullptr, nullptr);
	if (_window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

	// activate opengl context
	glfwMakeContextCurrent(_window);

	// glew is there to use the correct version for all functions
	glewExperimental = GL_TRUE;
	glewInit();

	check_glstate("Window successfully created", true);
}

void Display::create_shaders( void )
{
	_pointsUpdateProgram = createShaderProgram("points_update_vertex", "", "");

	glBindAttribLocation(_pointsUpdateProgram, POSATTRIB, "position");
	glBindAttribLocation(_pointsUpdateProgram, SPDATTRIB, "velocity");

	const GLchar *feedbackVaryings[] = {"Position", "Velocity"};
	glTransformFeedbackVaryings(_pointsUpdateProgram, 2, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(_pointsUpdateProgram);
	glUseProgram(_pointsUpdateProgram);

	check_glstate("points update shader program successfully created", true);

	_pointsRenderProgram = createShaderProgram("points_render_vertex", "", "points_render_fragment");

	glBindFragDataLocation(_pointsRenderProgram, 0, "outColor");

	glBindAttribLocation(_pointsRenderProgram, POSATTRIB, "position");
	glBindAttribLocation(_pointsRenderProgram, SPDATTRIB, "velocity");

	glLinkProgram(_pointsRenderProgram);
	glUseProgram(_pointsRenderProgram);

	check_glstate("points render shader program successfully created", true);

	_shaderProgram = createShaderProgram("vertex", "", "fragment");

	glBindFragDataLocation(_shaderProgram, 0, "outColor");

	glBindAttribLocation(_shaderProgram, POSATTRIB, "position");
	glBindAttribLocation(_shaderProgram, RADATTRIB, "radius");

	glLinkProgram(_shaderProgram);
	glUseProgram(_shaderProgram);

	check_glstate("shader program successfully created", true);
}

void Display::setup_communication_shaders( void )
{
	_uniDeltaT = glGetUniformLocation(_pointsUpdateProgram, "deltaTime");

	_uniPZoom = glGetUniformLocation(_pointsRenderProgram, "zoom");
	_uniPCenter = glGetUniformLocation(_pointsRenderProgram, "center");

	_uniZoom = glGetUniformLocation(_shaderProgram, "zoom");
	_uniCenter = glGetUniformLocation(_shaderProgram, "center");
	_uniMaxRadius = glGetUniformLocation(_shaderProgram, "maxRadius");
	_uniBigColor = glGetUniformLocation(_shaderProgram, "bigColor");
	_uniSmallColor = glGetUniformLocation(_shaderProgram, "smallColor");

	glGenVertexArrays(2, _vaos);
	glGenBuffers(2, _vbos);

	check_glstate("\nCommunication with shader programs successfully established", true);
}

void Display::setup_array_buffer( void )
{
	size_t pSize = _update_vertices.size();
	if (!pSize) return ;

	glBindVertexArray(_vaos[BUFFER::POINTS]);

	glBindBuffer(GL_ARRAY_BUFFER, _vbos[BUFFER::POINTS]);
	// std::cout << "vSize " << vSize << std::endl;
	glBufferData(GL_ARRAY_BUFFER, 4 * pSize * sizeof(GLfloat), &_update_vertices[0].v, GL_STATIC_DRAW);

	glEnableVertexAttribArray(POSATTRIB);
	glVertexAttribPointer(POSATTRIB, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(SPDATTRIB);
	glVertexAttribPointer(SPDATTRIB, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

	check_glstate("Display::setup_array_buffer - update vertices", false);

	size_t vSize = _vertices.size();
	if (!vSize) return ;

	glBindVertexArray(_vaos[BUFFER::TRIANGLES]);

	glBindBuffer(GL_ARRAY_BUFFER, _vbos[BUFFER::TRIANGLES]);
	// std::cout << "vSize " << vSize << std::endl;
	glBufferData(GL_ARRAY_BUFFER, 3 * vSize * sizeof(GLfloat), &_vertices[0].v, GL_STATIC_DRAW);

	glEnableVertexAttribArray(POSATTRIB);
	glVertexAttribPointer(POSATTRIB, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(RADATTRIB);
	glVertexAttribPointer(RADATTRIB, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

	check_glstate("Display::setup_array_buffer", false);
}

void Display::setup_delaunay( void )
{
	_update_vertices.clear();
	_vertices.clear();
	_points.clear();
	for (int i = 0; i < _nb_points; ++i) {
		Vertex point(Vertex(1000.0f * Random::randomFloat(_seed) - 500.0f, 1000.0f * Random::randomFloat(_seed) - 500.0f));
		// _points.push_back(Vertex(2.0f * Random::randomFloat(_seed) - 1.0f, 2.0f * Random::randomFloat(_seed) - 1.0f));
		float vx = 2.0f * Random::randomFloat(_seed) - 1.0f, vy = 2.0f * Random::randomFloat(_seed) - 1.0f;
		_update_vertices.push_back({point, Vertex((vx > 0) ? 50.0f + vx * 10.0f : -50.0f + vx * 10.0f, (vy > 0) ? 50.0f + vy * 10.0f : -50.0f + vy * 10.0f)});
		_points.push_back(point);
		// _points.push_back(Vertex(0.5f * Random::randomFloat(_seed) - 0.25f, 0.5f * Random::randomFloat(_seed) - 0.25f));
	}
	// (void)_seed;
	// _points.push_back(Vertex(0, 0));
	// _points.push_back(Vertex(0, 1));
	// _points.push_back(Vertex(1, 0.5));
	// vertices.push_back(Vertex(-1, 0.5));

	_delaunay = triangulate(_points);

	float maxRadius = 0;
	for (auto &t : _delaunay) {
		// float radius = t.getRadius();
		float radius = std::abs((t.getV1().getX() - t.getV0().getX()) * (t.getV2().getY() - t.getV1().getY())
				- (t.getV2().getX() - t.getV1().getX()) * (t.getV1().getY() - t.getV0().getY())) * 0.5f;
		// std::cout << "radius of triangle: " << radius << std::endl;
		if (radius > maxRadius) maxRadius = radius;
		_vertices.push_back({t.getV0(), radius});
		_vertices.push_back({t.getV1(), radius});
		_vertices.push_back({t.getV2(), radius});
	}
	glUseProgram(_shaderProgram);
	glUniform1f(_uniMaxRadius, maxRadius);
	setup_array_buffer();
}

void Display::reset_delaunay( void )
{
	_points.clear();
	_vertices.clear();

	for (auto &point : _update_vertices) {
		_points.push_back(point.v);
	}

	_delaunay = triangulate(_points);

	float maxRadius = 0;
	for (auto &t : _delaunay) {
		// float radius = t.getRadius();
		float radius = std::abs((t.getV1().getX() - t.getV0().getX()) * (t.getV2().getY() - t.getV1().getY())
				- (t.getV2().getX() - t.getV1().getX()) * (t.getV1().getY() - t.getV0().getY())) * 0.5f;
		// std::cout << "radius of triangle: " << radius << std::endl;
		if (radius > maxRadius) maxRadius = radius;
		_vertices.push_back({t.getV0(), radius});
		_vertices.push_back({t.getV1(), radius});
		_vertices.push_back({t.getV2(), radius});
	}
	glUseProgram(_shaderProgram);
	glUniform1f(_uniMaxRadius, maxRadius);
		
	size_t vSize = _vertices.size();
	if (!vSize) return ;

	glBindVertexArray(_vaos[BUFFER::TRIANGLES]);

	glBindBuffer(GL_ARRAY_BUFFER, _vbos[BUFFER::TRIANGLES]);
	// std::cout << "vSize " << vSize << std::endl;
	glBufferData(GL_ARRAY_BUFFER, 3 * vSize * sizeof(GLfloat), &_vertices[0].v, GL_STATIC_DRAW);

	glEnableVertexAttribArray(POSATTRIB);
	glVertexAttribPointer(POSATTRIB, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(RADATTRIB);
	glVertexAttribPointer(RADATTRIB, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

	check_glstate("Display::reset_delaunay", false);
}

void Display::handleInputs( void )
{
	if (_gui->keyboardControl()) return ;

	if (glfwGetKey(_window, GLFW_KEY_R) == GLFW_PRESS) {
		setup_delaunay();
	}

	if (_input_released && glfwGetKey(_window, GLFW_KEY_F3) == GLFW_PRESS) {
		_input_released = false;
		if (_gui->createWindow(-1, "Debug window", {20, 20}, {270, 150})) {
			_gui->addVarFloat("", &_deltaTime, "ms this frame");
			_gui->addVarInt("", &_fps, " FPS");
			_gui->addBool("update points", &_update_points);
			_gui->addBool("draw points", &_draw_points);
			_gui->addBool("draw delaunay", &_draw_delaunay);
			_gui->addSliderInt("points", &_nb_points, 3, 2500);
			_gui->addSliderFloat("Speed multiplier", &_speed_multiplier, 0.0f, 3.0f);
			_gui->addSliderFloat("zoom", &_zoom, 0.5f, 5.0f);
			_gui->addSliderFloat("center x", &_center[0], -500.0f, 500.0f);
			_gui->addSliderFloat("center y", &_center[1], -500.0f, 500.0f);
			_gui->addColor("big color", {&_bigCol[0], &_bigCol[1], &_bigCol[2], &_bigCol[3]});
			_gui->addColor("small color", {&_smallCol[0], &_smallCol[1], &_smallCol[2], &_smallCol[3]});
			_gui->addButton("RANDOMIZE", gui_randomize_callback);
		}
	} else if (glfwGetKey(_window, GLFW_KEY_F3) == GLFW_RELEASE) {
		_input_released = true;
	}
}

void Display::render( void )
{
	if (_update_points) {
		glBindVertexArray(_vaos[BUFFER::POINTS]);
		glUseProgram(_pointsUpdateProgram);
		glUniform1f(_uniDeltaT, _speed_multiplier * _deltaTime / 1000);

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _vbos[BUFFER::POINTS]);
		glEnable(GL_RASTERIZER_DISCARD); // we don't render anything

		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, _update_vertices.size());
		glEndTransformFeedback();

		// read update's output and gen new delaunay
		glFlush();
		glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 4 * sizeof(GLfloat) * _update_vertices.size(), &_update_vertices[0].v);
		reset_delaunay();

		glDisable(GL_RASTERIZER_DISCARD);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, NULL); // unbind transform feedback buffer
	}

	if (_draw_points) {
		glBindVertexArray(_vaos[BUFFER::POINTS]);
		glUseProgram(_pointsRenderProgram);
		glUniform1f(_uniPZoom, _zoom);
		glUniform2fv(_uniPCenter, 1, &_center[0]);
		glDrawArrays(GL_POINTS, 0, _update_vertices.size());
	}

	if (_draw_delaunay) {
		glBindVertexArray(_vaos[BUFFER::TRIANGLES]);
		glUseProgram(_shaderProgram);
		glUniform1f(_uniZoom, _zoom);
		glUniform2fv(_uniCenter, 1, &_center[0]);
		glUniform4fv(_uniBigColor, 1, &_bigCol[0]);
		glUniform4fv(_uniSmallColor, 1, &_smallCol[0]);
		glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
	}

	check_glstate("render", false);
}

void Display::main_loop( void )
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSwapInterval(1);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	set_display_callback(this, _gui);
	// glfwSetWindowSizeCallback(_window, window_size_callback);
	// glfwSetWindowPosCallback(_window, window_pos_callback);
	glfwSetCursorPosCallback(_window, cursor_pos_callback);
	glfwSetMouseButtonCallback(_window, mouse_button_callback);
	glfwSetCharCallback(_window, INPUT::character_callback);
	glfwSetWindowRefreshCallback(_window, window_refresh_callback);
	_gui->setWindowSize(_winWidth, _winHeight);

	check_glstate("setup done, entering main loop\n", true);

	double lastTime = glfwGetTime(), previousFrame = lastTime - 0.5, lastGameTick = lastTime;
	int nbFrames = 0, nbTicks = 0;
	_fps = 0;

	while (!glfwWindowShouldClose(_window)) {
		if (glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(_window, GL_TRUE);
			continue ;
		}

		handleInputs();

		double currentTime = glfwGetTime();
		_deltaTime = (currentTime - previousFrame) * 1000;
		++nbFrames;
		if (currentTime - lastTime >= 1.0) {
			// std::cout << "FPS: " << nbFrames << ", " << _nb_parts << " parts. current_core " << _current_core << "/" << _cores.size() << std::endl;
			_fps = nbFrames;
			nbFrames = 0;
			_tps = nbTicks;
			nbTicks = 0;
			lastTime += 1.0;
		}
		if (currentTime - lastGameTick >= TICK) {
			++nbTicks;
			lastGameTick += TICK;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render();
		_gui->render();
		glfwSwapBuffers(_window);
		glfwPollEvents();
		previousFrame = currentTime;
	}
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Display::setWindowSize( int width, int height )
{
	_winWidth = width;
	_winHeight = height;
	_gui->setWindowSize(width, height);
}

void Display::start( void )
{
	setup_window();
	create_shaders();
	setup_communication_shaders();
	setup_delaunay();
	_gui->start(_window);
	main_loop();
}
