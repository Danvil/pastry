#include <pastry/pastry.hpp>
#include "SOIL2/SOIL2.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <ctime>

namespace pastry
{
	void render_group::add(const renderling_ptr& ri)
	{
		items_.push_back(ri);
	}

	void render_group::remove(const renderling_ptr& ri)
	{
		auto it = std::find(items_.begin(), items_.end(), ri);
		if(it != items_.end()) {
			items_.erase(it);
		}
	}

	void render_group::update(float t, float dt)
	{
		// iterate over a copy to allow add/remove during update
		std::vector<renderling_ptr> items_copy = items_;
		for(const auto& i : items_copy) {
			i->update(t, dt);
		}
	}

	void render_group::render()
	{
		for(const auto& i : items_) {
			i->render();
		}
	}

	struct engine
	{
	private:
		GLFWwindow* window;
		render_group_ptr scene_;
	public:
		engine();
		~engine();
		float get_current_time() const;
		void run();
		const render_group_ptr& get_scene() const { return scene_; }
		void set_scene(const render_group_ptr& scene) { scene_ = scene; }
	};

	engine::engine()
	{
		window = 0;

		// initialize glfw
		if(!glfwInit()) {
			std::cerr << "ERROR: glfwInit failed!" << std::endl;
			return;
		}

		glfwWindowHint(GLFW_DEPTH_BITS, 16);

		// create window (windowed)
		window = glfwCreateWindow(640, 480, "pastry", NULL, NULL);
		if(!window) {
			std::cerr << "ERROR: glfwCreateWindow failed!" << std::endl;
			return;
		}

		// make the windows context current
		glfwMakeContextCurrent(window);

		// set time taken by glfwSwapBuffers
		glfwSwapInterval(1);

		glfwSetTime(0);

		// initializing GLEW
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if(GLEW_OK != err) {
			std::cerr << "ERROR: glewInit failed!" << std::endl;
			std::cerr << glewGetErrorString(err) << std::endl;
			window = 0;
			return;
		}
//		std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;

		scene_ = std::make_shared<render_group>();
	}

	engine::~engine()
	{
		glfwTerminate();
	}

	float engine::get_current_time() const
	{
		//return std::chrono::system_clock::now().
		//return static_cast<float>(clock()) / (float)CLOCKS_PER_SEC;
		return glfwGetTime();
	}

	void engine::run()
	{
		if(window == 0) {
			std::cerr << "ERROR: Did not call 'initialize' or error occurred!" << std::endl;
			return;
		}

		float last_time = get_current_time();
		unsigned int num_frames = 0;
		float fps = -1;

		// main loop (until window is closed by user)
		while (!glfwWindowShouldClose(window))
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
			glfwSwapBuffers(window);
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
				glfwSetWindowTitle(window, buffer);
			}
		}
	}

	std::shared_ptr<engine> s_engine;

	namespace sprites { void initialize(); }

	void initialize()
	{
		s_engine = std::make_shared<engine>();
		sprites::initialize();
	}

	void run()
	{
		s_engine->run();
	}

	void add_renderling(const renderling_ptr& r)
	{
		s_engine->get_scene()->add(r);
	}

	void remove_renderling(const renderling_ptr& r)
	{
		s_engine->get_scene()->remove(r);
	}

	texture load_texture(const std::string& fn)
	{
		texture tex;
		GLuint q = SOIL_load_OGL_texture(
			fn.data(),
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
		);
		if(q == 0) {
			std::cerr << "ERROR in load_texture: Failed to load image '" << fn << "'" << std::endl;
		}
		tex.id_set(q);
		tex.width_ = tex.get_width();
		tex.height_ = tex.get_height();
		return tex;
	}

}
