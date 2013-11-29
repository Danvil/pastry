#include "engine.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>

namespace pastry {

std::shared_ptr<engine> engine::s_engine;

engine::engine()
{
	window_ = 0;

	// initialize glfw
	if(!glfwInit()) {
		std::cerr << "ERROR: glfwInit failed!" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_DEPTH_BITS, 16);

	// create window (windowed)
	window_ = glfwCreateWindow(512, 512, "pastry", NULL, NULL);
	if(!window_) {
		std::cerr << "ERROR: glfwCreateWindow failed!" << std::endl;
		return;
	}

	// make the windows context current
	glfwMakeContextCurrent(window_);

	// set time taken by glfwSwapBuffers
	glfwSwapInterval(1);

	glfwSetTime(0);

	// initializing GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(GLEW_OK != err) {
		std::cerr << "ERROR: glewInit failed!" << std::endl;
		std::cerr << glewGetErrorString(err) << std::endl;
		window_ = 0;
		return;
	}
//		std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;

	scene_ = std::make_shared<render_group>();
}

engine::~engine()
{
	glfwTerminate();
}

Eigen::Vector2f engine::get_cursor_pos() const
{
	double x, y;
	glfwGetCursorPos(window_, &x, &y);
	return {x,y};
}

bool engine::is_key_pressed(int key) const
{
	int q = glfwGetKey(window_, key);
	return q == GLFW_PRESS;
}

bool engine::is_mouse_button_pressed(int button) const
{
	int q = glfwGetMouseButton(window_, button);
	return q == GLFW_PRESS;
}


float engine::get_current_time() const
{
	//return std::chrono::system_clock::now().
	//return static_cast<float>(clock()) / (float)CLOCKS_PER_SEC;
	return glfwGetTime();
}

void engine::run()
{
	if(window_ == 0) {
		std::cerr << "ERROR: Did not call 'initialize' or error occurred!" << std::endl;
		return;
	}

	float last_time = get_current_time();
	unsigned int num_frames = 0;
	float fps = -1;

	// main loop (until window is closed by user)
	while (!glfwWindowShouldClose(window_))
	{
		// update stuff
		float current_time = get_current_time();
		float delta_time = current_time - last_time;
		last_time = current_time;
		scene_->update(current_time, delta_time);
		
		// render stuff
		glClear(GL_COLOR_BUFFER_BIT);
		scene_->render();
		glFlush();

		// housekeeping
		glfwSwapBuffers(window_);
		glfwPollEvents();

		// fps
		num_frames ++;
		if(num_frames >= 100 && delta_time >= 1.0f) {
			float current_fps = static_cast<float>(num_frames) / delta_time;
			num_frames = 0;
			// update fps
			// if(fps < 0) fps = current_fps;
			// fps += 0.40f * (current_fps - fps);
			fps = current_fps;
			// set window title
			char buffer[512];
			sprintf(buffer, "pastry - %.0f fps\0", fps);
			glfwSetWindowTitle(window_, buffer);
		}
	}
}

}
