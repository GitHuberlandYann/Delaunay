#include "Display.hpp"
#include "random.hpp"
// #include "callbacks.hpp"

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT), _seed(1503)
{
	// _gui = new Gui();
}

Display::~Display( void )
{
	std::cout << "Destructor of display called" << std::endl;

	glDeleteProgram(_shaderProgram);

	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);

	glfwMakeContextCurrent(NULL);
    glfwTerminate();

	// delete _gui;

	check_glstate("Display successfully destructed", true);
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

void Display::setup_window( void )
{
	// glfwSetErrorCallback(error_callback);
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
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);

	check_glstate("\nCommunication with shader program successfully established", true);
}

void Display::setup_array_buffer( void )
{
	size_t vSize = _vertices.size();
	if (!vSize) return ;

	glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	std::cout << "vSize " << vSize << std::endl;
	glBufferData(GL_ARRAY_BUFFER, 3 * vSize * sizeof(GLfloat), &_vertices[0].v, GL_STATIC_DRAW);

	glEnableVertexAttribArray(POSATTRIB);
	glVertexAttribPointer(POSATTRIB, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(RADATTRIB);
	glVertexAttribPointer(RADATTRIB, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));

	check_glstate("Display::setup_array_buffer", false);
}

void Display::setup_delaunay( void )
{
	for (int i = 0; i < 100; ++i) {
		_points.push_back(Vertex(2.0f * Random::randomFloat(_seed) - 1.0f, 2.0f * Random::randomFloat(_seed) - 1.0f));
	}
	// (void)_seed;
	// _points.push_back(Vertex(0, 0));
	// _points.push_back(Vertex(0, 1));
	// _points.push_back(Vertex(1, 0.5));
	// vertices.push_back(Vertex(-1, 0.5));

	_delaunay = triangulate(_points);

	for (auto &t : _delaunay) {
		float radius = t.getRadius();
		// std::cout << "radius of triangle: " << radius << std::endl;
		_vertices.push_back({t.getV0(), radius});
		_vertices.push_back({t.getV1(), radius});
		_vertices.push_back({t.getV2(), radius});
	}
	setup_array_buffer();

	// for (auto &t : delaunay) {
	// 	// std::cout << "sizeof triang is " << sizeof(t) << std::endl;
	// 	std::cout << "delaunay triangle at " << t.getV0().getX() << ", " << t.getV0().getY() << " - " << t.getV1().getX() << ", " << t.getV1().getY() << " - " << t.getV2().getX() << ", " << t.getV2().getY() << std::endl;
	// }
}

void Display::handleInputs( void )
{
}

void Display::render( void )
{
	glUseProgram(_shaderProgram);
	glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
	// glDrawArrays(GL_POINTS, 0, num_part);

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
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
		glfwSwapBuffers(_window);
		glfwPollEvents();
		previousFrame = currentTime;
	}
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Display::start( void )
{
	setup_window();
	create_shaders();
	setup_communication_shaders();
	setup_delaunay();
	main_loop();
}
