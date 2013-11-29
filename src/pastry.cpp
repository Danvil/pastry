#include <pastry/pastry.hpp>
#include "engine.hpp"
#include "SOIL2/SOIL2.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <ctime>

namespace pastry {

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

void initialize_sprites();
void initialize_text();

void initialize()
{
	engine::s_engine = std::make_shared<engine>();
	initialize_sprites();
	initialize_text();
}

void run()
{
	engine::s_engine->run();
}

void add_renderling(const renderling_ptr& r)
{
	engine::s_engine->get_scene()->add(r);
}

void remove_renderling(const renderling_ptr& r)
{
	engine::s_engine->get_scene()->remove(r);
}

bool is_key_pressed(int key)
{
	return engine::s_engine->is_key_pressed(key);
}

bool is_left_mouse_button_pressed()
{
	return engine::s_engine->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
}

bool is_right_mouse_button_pressed()
{
	return engine::s_engine->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
}

bool is_middle_mouse_button_pressed()
{
	return engine::s_engine->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_MIDDLE);
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

void save_texture(const texture& tex, const std::string& fn)
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

}
