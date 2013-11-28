#ifndef INCLUDED_PASTRY_PASTRY_HPP
#define INCLUDED_PASTRY_PASTRY_HPP

#include "pastry_gl.hpp"
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>

typedef struct GLFWwindow GLFWwindow;

namespace pastry {

// ----- THE START -------------------------------------------------------------

// ----- BASIC RENDERLINGS -----------------------------------------------------

class renderling
{
public:
	virtual void update(float t, float dt) {}
	virtual void render() {}
};

typedef std::shared_ptr<renderling> renderling_ptr;

class functor_renderling
: public renderling
{
public:
	typedef std::function<void()> func_render_t;
	typedef std::function<void(float,float)> func_update_t;
	func_render_t f_render;
	func_update_t f_update;
public:
	functor_renderling(func_render_t fr, func_update_t fu)
	: f_render(fr), f_update(fu) {}
	functor_renderling(func_render_t fr)
	: f_render(fr) {}
	void update(float t, float dt) {
		if(f_update) f_update(t,dt);
	}
	void render() {
		if(f_render) f_render();
	}
};

typedef std::shared_ptr<functor_renderling> functor_renderling_ptr;

inline functor_renderling_ptr create_functor_renderling(
		functor_renderling::func_render_t fr) {
	return std::make_shared<functor_renderling>(fr);
}

inline functor_renderling_ptr create_functor_renderling(
	functor_renderling::func_render_t fr,
	functor_renderling::func_update_t fu) {
	return std::make_shared<functor_renderling>(fr, fu);
}

class render_group : public renderling
{
private:
	std::vector<renderling_ptr> items_;
public:
	void add(const renderling_ptr& ri);
	void remove(const renderling_ptr& ri);
	void update(float t, float dt);
	void render();
};

typedef std::shared_ptr<render_group> render_group_ptr;

// ----- GENERAL RENDER SETUP --------------------------------------------------

/** Initialized pastry OpenGL engine. Call this before doing everything else. */
void initialize();

/** Run the main loop. Call this once your initial setup is finished. */
void run();

/** Adds a renderling to the scene */
void add_renderling(const renderling_ptr& r);

// workaround for gcc 4.6
template<typename T>
void add_renderling(const std::shared_ptr<T>& r) {
	renderling_ptr rr = r;
	add_renderling(rr);
}

/** Removes a renderling to the scene */
void remove_renderling(const renderling_ptr& r);

/** Adds a functor renderling to the scene and returns it */
inline functor_renderling_ptr add_renderling(
		functor_renderling::func_render_t fr) {
	functor_renderling_ptr f = create_functor_renderling(fr);
	add_renderling(f);
	return f;
}

/** Adds a functor renderling to the scene and returns it */
inline functor_renderling_ptr add_renderling(
		functor_renderling::func_render_t fr,
		functor_renderling::func_update_t fu) {
	functor_renderling_ptr f = create_functor_renderling(fr, fu);
	add_renderling(f);
	return f;
}

// ----- TEXTURE LOADING -------------------------------------------------------

texture load_texture(const std::string& fn);

void save_texture(const texture& tex, const std::string& fn);

// ----- TEXT RENDERING --------------------------------------------------------

void render_text(float x, float y, const std::string& txt);

// ----- SPRITES ---------------------------------------------------------------

namespace detail
{
	struct def_sprite
	{
		std::string tag;
		float u_px, v_px;
		float su_px, sv_px;
	};

	struct def_sheet
	{
		std::string tag;
		texture tex;
		std::vector<def_sprite> sprites;
	};

	struct def_anim_frame
	{
		std::string sprite_tag;
		float dt;
	};

	struct def_anim
	{
		std::string tag;
		std::vector<def_anim_frame> frames;
	};

	struct sprite_vertex
	{
		float x, y;
		float u, v;
	};
}

struct sprite
{
	std::string tag;
	float x, y;
	float sx, sy;
	float t;
};

typedef std::shared_ptr<sprite> sprite_ptr;

void add_sprite_sheet(const detail::def_sheet& sheet);

void add_sprite_animation(const detail::def_anim& anim);

class sprite_group
: public renderling
{
public:
	sprite_group();
	~sprite_group();
	sprite_ptr add_sprite(const std::string& tag);
	void remove_sprite(const sprite_ptr& s);
	void update(float t, float dt);
	void render();
private:
	std::vector<sprite_ptr> sprites_;
	std::map<std::string, std::vector<detail::sprite_vertex>> vertices_;
};

// ----- THE END ---------------------------------------------------------------

}
#endif
