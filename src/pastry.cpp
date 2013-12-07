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
		//std::cout << "update " << i.first << ": " << i.second << std::endl;
		i.second->update(t, dt);
	}
}

void render_group::render()
{
	for(const auto& i : items_) {
		//std::cout << "render " << i.first << ": " << i.second << std::endl;
		i.second->render();
	}
}

// ----- GENERAL RENDER SETUP --------------------------------------------------

GLFWwindow* g_window = 0;
render_group_ptr g_scene;
int g_fb_width = 0;
int g_fb_height = 0;
bool g_fb_changed = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	g_fb_changed = true;
	g_fb_width = width;
	g_fb_height = height;
	glViewport(0, 0, g_fb_width, g_fb_height); // TODO should we or the user do this?
}

bool fb_has_changed()
{
	return g_fb_changed;
}

void fb_get_dimensions(int& width, int& height)
{
	width = g_fb_width;
	height = g_fb_height;
}

void initialize_sprites();
void initialize_text();
void postfx_init();

void initialize()
{
	g_window = 0;
	g_fb_width = 512;
	g_fb_height = 512;

	// initialize glfw
	if(!glfwInit()) {
		std::cerr << "ERROR: glfwInit failed!" << std::endl;
		return;
	}

	glfwWindowHint(GLFW_DEPTH_BITS, 16);

	// create window (windowed)
	g_window = glfwCreateWindow(g_fb_width, g_fb_height, "pastry", NULL, NULL);
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

	glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);

	g_scene = std::make_shared<render_group>();

	initialize_sprites();

	initialize_text();

	postfx_init();
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

		g_fb_changed = false;

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
	// see http://www.songho.ca/opengl/gl_projectionmatrix.html
	Eigen::Matrix4f m;
	m <<
		+2.0f/(r-l), 0, 0, -(r+l)/(r-l),
		0, +2.0f/(t-b), 0, -(t+b)/(t-b),
		0, 0, -2.0f/(f-n), -(f+n)/(f-n),
		0, 0, 0, 1;
	return m;
}

Eigen::Matrix4f math_orthogonal_projection(float w, float h, float n, float f, bool ydown)
{
	if(ydown)
		return math_orthogonal_projection(0, w, 0, h, n, f);
	else
		return math_orthogonal_projection(0, w, h, 0, n, f);
}

Eigen::Matrix4f math_orthogonal_projection(float n, float f, bool ydown)
{
	return math_orthogonal_projection(g_fb_width, g_fb_height, n, f, ydown);
}

Eigen::Matrix4f math_orthogonal_projection(float s, float n, float f, bool ydown)
{
	float aspect = static_cast<float>(g_fb_height)/static_cast<float>(g_fb_width);
	aspect *= (ydown ? 1.0f : -1.0f);
	return pastry::math_orthogonal_projection(
		-s, +s,
		-s*aspect, +s*aspect,
		n, f);
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

void math_backproject(const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view,
	Eigen::Vector3f& ray_a, Eigen::Vector3f& ray_u)
{
	float z_near = -1.0f;
	// mouse in clipping coordinates
	Eigen::Vector2f mouse_screen = mouse_get_position();
	float mx = 2.0f*mouse_screen[0]/static_cast<float>(g_fb_width) - 1.0f;
	float my = -(2.0f*mouse_screen[1]/static_cast<float>(g_fb_height) - 1.0f);
	// un-project: get two points which are projected onto the mouse position
	//             by setting the clipped z value to -1 (close) and +1 (far)
	Eigen::Vector4f pa = proj.inverse() * Eigen::Vector4f{mx,my,-1,1};
	pa /= pa[3]; // normalize
	Eigen::Vector4f pb = proj.inverse() * Eigen::Vector4f{mx,my,+1,1}; 
	pb /= pb[3]; // normalize
	// transform from camera to world coordinates
	Eigen::Matrix4f view_inv = view.inverse();
	Eigen::Vector4f cpa = view_inv * pa;
	Eigen::Vector4f cpb = view_inv * pb;
	ray_a = cpa.block<3,1>(0,0);
	ray_u = (cpb.block<3,1>(0,0) - cpa.block<3,1>(0,0)).normalized();
}

Eigen::Vector3f math_backproject(const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, float z)
{
	Eigen::Vector3f a, u;
	math_backproject(proj, view, a, u);
	// solve (a + s*u)_z == z
	float s = (z - a[2]) / u[2];
	return a + s * u;
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
