#ifndef INCLUDED_PASTRY_ENGINE_HPP
#define INCLUDED_PASTRY_ENGINE_HPP

#include <pastry/pastry.hpp>
#include <Eigen/Dense>
#include <memory>

typedef struct GLFWwindow GLFWwindow;

namespace pastry
{

	struct engine
	{
	public:
		static std::shared_ptr<engine> s_engine;

	private:
		GLFWwindow* window_;
		render_group_ptr scene_;

	public:
		engine();
		~engine();

		GLFWwindow* window() const { return window_; }

		Eigen::Vector2f get_cursor_pos() const;

		bool is_key_pressed(int key) const;

		bool is_mouse_button_pressed(int button) const;

		float get_current_time() const;

		void run();

		const render_group_ptr& get_scene() const { return scene_; }

		void set_scene(const render_group_ptr& scene) { scene_ = scene; }
	};

}

#endif
