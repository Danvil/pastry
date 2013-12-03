#include <pastry/pastry.hpp>
#include "dep/SOIL2/SOIL2.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <ctime>

namespace pastry {

// ----- BASIC RENDERLINGS -----------------------------------------------------

void render_group::add(const renderling_ptr& r, int o)
{
	items_.insert({o,r});
}

void render_group::remove(const renderling_ptr& r)
{
	typedef std::multimap<int,renderling_ptr>::iterator it_t;
	for(it_t it=items_.begin(); it!=items_.end(); ) {
		it_t a = it++; // iterators are invalidated by erase
		if(a->second == r) {
			items_.erase(a);
		}
	}
}

void render_group::update(float t, float dt)
{
	// iterate over a copy to allow add/remove during update
	std::multimap<int,renderling_ptr> items_copy = items_;
	for(const auto& i : items_copy) {
		i.second->update(t, dt);
	}
}

void render_group::render()
{
	for(const auto& i : items_) {
		i.second->render();
	}
}

// ----- GENERAL RENDER SETUP --------------------------------------------------

GLFWwindow* g_window = 0;
render_group_ptr g_scene;

void initialize_sprites();
void initialize_text();

void initialize()
{
	g_window = 0;

	// initialize glfw
	if(!glfwInit()) {
		std::cerr << "ERROR: glfwInit failed!" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_DEPTH_BITS, 16);

	// create window (windowed)
	g_window = glfwCreateWindow(512, 512, "pastry", NULL, NULL);
	if(!g_window) {
		std::cerr << "ERROR: glfwCreateWindow failed!" << std::endl;
		return;
	}

	// make the windows context current
	glfwMakeContextCurrent(g_window);

	// set time taken by glfwSwapBuffers
	glfwSwapInterval(1);

	glfwSetTime(0);

	// initializing GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(GLEW_OK != err) {
		std::cerr << "ERROR: glewInit failed!" << std::endl;
		std::cerr << glewGetErrorString(err) << std::endl;
		g_window = 0;
		return;
	}
//		std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;

	g_scene = std::make_shared<render_group>();

	initialize_sprites();

	initialize_text();
}

void run()
{
	if(g_window == 0) {
		std::cerr << "ERROR: Did not call 'initialize' or error occurred!" << std::endl;
		return;
	}

	float last_time = glfwGetTime();
	unsigned int num_frames = 0;
	float fps_delta_time = 0.0f;
	float fps = -1;

	// main loop (until window is closed by user)
	while (!glfwWindowShouldClose(g_window))
	{
		// update stuff
		float current_time = glfwGetTime();
		float delta_time = current_time - last_time;
		last_time = current_time;
		g_scene->update(current_time, delta_time);
		
		// render stuff
		glClear(GL_COLOR_BUFFER_BIT);
		g_scene->render();
		glFlush();

		// housekeeping
		glfwSwapBuffers(g_window);
		glfwPollEvents();

		// fps
		num_frames ++;
		fps_delta_time += delta_time;
		if(num_frames >= 100 && fps_delta_time >= 1.0f) {
			float current_fps = static_cast<float>(num_frames) / fps_delta_time;
			num_frames = 0;
			fps_delta_time = 0.0f;
			// update fps
			// if(fps < 0) fps = current_fps;
			// fps += 0.40f * (current_fps - fps);
			fps = current_fps;
			// set window title
			char buffer[512];
			sprintf(buffer, "pastry - %.0f fps\0", fps);
			glfwSetWindowTitle(g_window, buffer);
		}
	}

	glfwDestroyWindow(g_window);
	glfwTerminate();
}

void scene_add(const renderling_ptr& r, int order)
{
	g_scene->add(r, order);
}

void scene_remove(const renderling_ptr& r)
{
	g_scene->remove(r);
}

// ----- TOOLS -----------------------------------------------------------------

Eigen::Matrix4f math_orthogonal_projection(float l, float r, float t, float b, float n, float f)
{
	Eigen::Matrix4f m;
	m <<
		+2.0f/(r-l), 0, 0, -(r+l)/(r-l),
		0, +2.0f/(t-b), 0, -(t+b)/(t-b),
		0, 0, -2.0f/(f-n), -(f+n)/(f-n),
		0, 0, 0, 1;
	return m;
}

Eigen::Matrix4f math_orthogonal_projection(float w, float h, float n, float f)
{
	return math_orthogonal_projection(0, w, 0, h, n, f);
}

Eigen::Matrix4f math_transform_2d(float x, float y, float theta)
{
	float st = std::sin(theta);
	float ct = std::cos(theta);
	Eigen::Matrix4f m;
	m <<
		 ct, +st, 0, x,
		-st,  ct, 0, y,
		  0,   0, 1, 0,
		  0,   0, 0, 1;
	return m;
}

// ----- INPUT HANDLING --------------------------------------------------------

bool key_is_pressed(int key)
{
	int q = glfwGetKey(g_window, key);
	return q == GLFW_PRESS;
}

bool is_mouse_button_pressed(int button)
{
	int q = glfwGetMouseButton(g_window, button);
	return q == GLFW_PRESS;
}

bool mouse_is_left_button_pressed()
{
	return is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
}

bool mouse_is_right_button_pressed()
{
	return is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
}

bool mouse_is_middle_button_pressed()
{
	return is_mouse_button_pressed(GLFW_MOUSE_BUTTON_MIDDLE);
}

Eigen::Vector2f mouse_get_position()
{
	double x, y;
	glfwGetCursorPos(g_window, &x, &y);
	return {x,y};
}

// ----- TEXTURE LOADING -------------------------------------------------------

texture texture_load(const std::string& fn)
{
	texture tex;
	GLuint q = SOIL_load_OGL_texture(
		fn.data(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
	);
	if(q == 0) {
		std::cerr << "ERROR in texture_load: Failed to load image '" << fn << "'" << std::endl;
	}
	tex.id_set(q);
	tex.width_ = tex.get_width();
	tex.height_ = tex.get_height();
	return tex;
}

void texture_save(const texture& tex, const std::string& fn)
{
	std::vector<unsigned char> img;
	if(tex.channels_ == 1) img = tex.get_image_red_ub();
	if(tex.channels_ == 3) img = tex.get_image_rgb_ub();
	SOIL_save_image(
		fn.data(),
		SOIL_SAVE_TYPE_PNG,
		tex.width(), tex.height(), tex.channels_,
		img.data()
	);
}

// ----- THE END ---------------------------------------------------------------

}
