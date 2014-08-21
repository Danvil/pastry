#pragma once

#include <pastry/gl.hpp>

namespace pastry {
namespace deferred {

class GBuffer
{
public:
	GBuffer();

	void update();

	void prePass();

	void startGeometryPass();
	void stopGeometryPass();

	void startLightPass();
	void stopLightPass();

	void startForwardPass();
	void stopForwardPass();

	void finalPass();

private:
	int width_, height_;
	pastry::texture_2d tex_final;
	pastry::texture_2d tex_position, tex_normal, tex_color, tex_material;
	pastry::texture_2d tex_depth;
	pastry::framebuffer fbo;

	int dbg_ = 0;

};

}}
