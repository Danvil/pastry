#ifndef INCLUDED_PASTRY_PASTRY_HPP
#define INCLUDED_PASTRY_PASTRY_HPP

#include "gl.hpp"
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include <GL/glew.h> // must be included before glfw header!
#include <GLFW/glfw3.h>

namespace pastry {

// ----- BASIC RENDERLINGS -----------------------------------------------------

#define PASTRY_DEFINE_PTR(A) typedef std::shared_ptr<A> A##_ptr;

/** Base class for everything renderable */
class renderling
{
public:
	virtual void update(float t, float dt) {}
	virtual void render() {}
};

PASTRY_DEFINE_PTR(renderling)

/** A stateless renderling */
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

PASTRY_DEFINE_PTR(functor_renderling)

/** Create a stateless renderling (render only) */
inline functor_renderling_ptr create_functor_renderling(
		functor_renderling::func_render_t fr) {
	return std::make_shared<functor_renderling>(fr);
}

/** Create a stateless renderling (render and update) */
inline functor_renderling_ptr create_functor_renderling(
	functor_renderling::func_render_t fr,
	functor_renderling::func_update_t fu) {
	return std::make_shared<functor_renderling>(fr, fu);
}

/** A group of renderlings
 * Default render and update order is the order of insertion.
 * You can specify an order explicitely when adding (default=0, smaller first)
 */
class render_group : public renderling
{
private:
	std::multimap<int,renderling_ptr> items_;
public:
	/** Adds a renderling with specific render order */
	void add(const renderling_ptr& r, int order=0);
	/** Removes a renderling (all instances) */
	void remove(const renderling_ptr& r);
	/** Updates all renderlings */
	void update(float t, float dt);
	/** Renders all renderlings */
	void render();
};

PASTRY_DEFINE_PTR(render_group)

// ----- GENERAL RENDER SETUP --------------------------------------------------

/** Initialized pastry OpenGL engine. Call this before doing everything else. */
void initialize();

/** Checks wether the framebuffer size changed since the last call to upate */
bool fb_has_changed();

/** Gets the size of the framebuffer */
void fb_get_dimensions(int& width, int& height);

/** Run the main loop. Call this once your initial setup is finished. */
void run();

/** Adds a renderling to the scene */
void scene_add(const renderling_ptr& r, int order=0);

// workaround for gcc 4.6
template<typename T>
void scene_add(const std::shared_ptr<T>& r, int order=0) {
	renderling_ptr rr = r;
	scene_add(rr, order);
}

/** Removes a renderling from the scene */
void scene_remove(const renderling_ptr& r);

/** Adds a functor renderling (render only) to the scene and returns it */
inline functor_renderling_ptr scene_add(
		functor_renderling::func_render_t fr) {
	functor_renderling_ptr f = create_functor_renderling(fr);
	scene_add(f);
	return f;
}

/** Adds a functor renderling (render and update) to the scene and returns it */
inline functor_renderling_ptr scene_add(
		functor_renderling::func_render_t fr,
		functor_renderling::func_update_t fu) {
	functor_renderling_ptr f = create_functor_renderling(fr, fu);
	scene_add(f);
	return f;
}

// ----- TOOLS -----------------------------------------------------------------

/** Create an orthogonal projection matrix */
Eigen::Matrix4f math_orthogonal_projection(float l, float r, float t, float b, float n, float f);

/** Create an orthogonal projection matrix
 * y=0 is on the bottome of the window unless ydown is set to true
 */
Eigen::Matrix4f math_orthogonal_projection(float w, float h, float n, float f, bool ydown=false);

/** Create an orthogonal projection matrix for the current resolution */
Eigen::Matrix4f math_orthogonal_projection(float n, float f, bool ydown=false);

Eigen::Matrix4f math_orthogonal_projection(float s, float n, float f, bool ydown=false);

/** Create a 2D transformation matrix */
Eigen::Matrix4f math_transform_2d(float x, float y, float theta);

void math_backproject(
	const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view,
	const Eigen::Vector2f& p,
	Eigen::Vector3f& a, Eigen::Vector3f& u);

Eigen::Vector3f math_backproject(
	const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view,
	const Eigen::Vector2f& p,
	float z);

// ----- INPUT HANDLING --------------------------------------------------------

/** Checks if a key is pressed right now */
bool key_is_pressed(int key);

/** Checks if a key down happend during last frame */
bool key_is_key_down(int key);

/** Checks if a key up happend during last frame */
bool key_is_key_up(int key);

/** Checks if left mouse button is pressed right now */
bool mouse_is_left_button_pressed();

/** Checks if right mouse button is pressed right now */
bool mouse_is_right_button_pressed();

/** Checks if middle mouse button is pressed right now */
bool mouse_is_middle_button_pressed();

/** Gets the position of the mouse cursor */
Eigen::Vector2f mouse_get_position();

// ----- TEXTURE LOADING -------------------------------------------------------

/** Loads an image from file into a texture (only some image formats supported) */
texture_base texture_load(const std::string& fn);

/** Saves an RGB or single channel texture to a file */
void texture_save(const texture_base& tex, const std::string& fn);

// ----- TEXT RENDERING --------------------------------------------------------

/** Loads a font (first to load is the default) */
void text_load_font(const std::string& name, const std::string& filename, float font_size=32.0f);

/** Renders white text at the specific location on the screen (in pixels) */
void text_render(float x, float y, const std::string& txt);

/** Renders colored and transparent text */
void text_render(float x, float y, const std::string& txt, const Eigen::Vector4f& rgba, const std::string& font);
void text_render(float x, float y, const std::string& txt, const Eigen::Vector4f& rgba);

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
		texture_base tex;
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
	/** The tag under which the sprite definition is found */
	std::string tag;
	/** Sprite position */
	float x, y;
	/** Sprite scale factors */
	float sx, sy;
	/** internal */
	float t;
};

PASTRY_DEFINE_PTR(sprite)

/** Adds a sprite sheet */
void sprites_add_sheet(const detail::def_sheet& sheet, bool filter_nearest=true);

/** Adds a sprite animation */
void sprites_add_animation(const detail::def_anim& anim);

/** A group of sprites */
class sprite_group
: public renderling
{
public:
	/** Creates a new sprite with given tag */
	sprite_ptr add_sprite(const std::string& tag);
	/** Removes the specified sprite */
	void remove_sprite(const sprite_ptr& s);
	/** Updates sprite animations preparse for rendering */
	void update(float t, float dt);
	/** Renders all sprites */
	void render();
private:
	std::vector<sprite_ptr> sprites_;
	std::map<std::string, std::vector<detail::sprite_vertex>> vertices_;
};

// ----- POST PROCESSING EFFECTS -----------------------------------------------

class post_effect
{
public:
	virtual std::string source() const { return "vec4 sfx(vec2 uv) { return sfx_read_fb(uv); }"; }
	virtual void update(float t, float dt, const program& spo) {}
};

PASTRY_DEFINE_PTR(post_effect)

post_effect_ptr postfx_add(const std::string& source);

typedef std::function<void(float,float,const pastry::program& spo)> func_postfx_update;

post_effect_ptr postfx_add(const std::string& source, func_postfx_update f);

void postfx_add(const post_effect_ptr& p);

void postfx_remove(const post_effect_ptr& p);

// ----- THE END ---------------------------------------------------------------

}
#endif
