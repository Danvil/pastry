#pragma once

#include <pastry/gl.hpp>

namespace pastry {
namespace deferred {

class GBuffer
{
public:
	GBuffer();

	void startGeometryPass();

	void stopGeometryPass();

	void update();

	void startLightPass();

	void stopLightPass();

	void finalPass();

private:
	int width_, height_;
	pastry::texture_base tex_position, tex_normal, tex_color;
	pastry::texture_base tex_final;
	pastry::texture_base tex_depth;
	pastry::framebuffer fbo;

	int dbg_ = 0;

};

}}
