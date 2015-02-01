#ifndef INCLUDED_PASTRY_PASTRY_HPP
#define INCLUDED_PASTRY_PASTRY_HPP

#include <pastry/gl.hpp>
#include <functional>
#include <map>
#include <string>
#include <vector>
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
void initialize(int width, int height, const std::string& titel);

/** Checks wether the framebuffer size changed since the last call to upate */
bool fb_has_changed();

/** Gets the size of the framebuffer */
void fb_get_dimensions(int& width, int& height);

/** Gets the aspect ratio of the framebuffer */
float fb_get_aspect();

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

Eigen::Matrix4f math_perspective_projection(float angle, float aspect, float n, float f);

/** Create a 2D transformation matrix */
Eigen::Matrix4f math_transform_2d(float x, float y, float theta);

Eigen::Matrix4f lookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up);

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

// ----- PRIMITVE RENDERING ---------------------------------------------------------------

void setPrimitiveColor(const Eigen::Vector3f& color);

void setCamera2D(const Eigen::Vector2f& center, float width);

void renderLine2D(const Eigen::Vector2f& start, const Eigen::Vector2f& end);

void renderCircle2D(const Eigen::Vector2f& center, float radius);

void renderBox2D(const Eigen::Vector2f& center, float angle, const Eigen::Vector2f& size);

void renderPolygon2D(const std::vector<Eigen::Vector2f>& points);

// ----- TEXTURE LOADING -------------------------------------------------------

/** Loads an image from file into a texture (only some image formats supported) */
texture_2d texture_load(const std::string& fn);

/** Loads a cube texture image from file into a cube texture (only some image formats supported) */
texture_cube_map texture_load_cube(const std::string& fn, const std::string& order);

/** Saves an RGB or single channel texture to a file */
void texture_save(const texture_2d& tex, const std::string& fn);

// ----- TEXT RENDERING --------------------------------------------------------

/** Loads a font (first to load is the default) */
void text_load_font(const std::string& name, const std::string& filename, float font_size=32.0f);

/** Renders white text at the specific location on the screen (in pixels) */
void text_render(float x, float y, const std::string& txt);

/** Renders colored and transparent text */
void text_render(float x, float y, const std::string& txt, const Eigen::Vector4f& rgba, const std::string& font);
void text_render(float x, float y, const std::string& txt, const Eigen::Vector4f& rgba);

// ----- THE END ---------------------------------------------------------------

}
#endif
