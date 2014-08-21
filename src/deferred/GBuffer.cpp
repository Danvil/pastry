#include <pastry/deferred/GBuffer.hpp>
#include <pastry/pastry.hpp>

namespace pastry {
namespace deferred {

GBuffer::GBuffer()
{
	pastry::fb_get_dimensions(width_, height_);
	fbo.bind();

	tex_position.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_position.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT0, tex_position);

	tex_normal.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_normal.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT1, tex_normal);

	tex_color.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_color.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT2, tex_color);

	tex_final.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_final.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT3, tex_final);

	tex_depth.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_depth.set_image_depth<float>(GL_DEPTH_COMPONENT32F, width_, height_);
	fbo.attach(GL_DEPTH_ATTACHMENT, tex_depth);

	fbo.unbind();
}

void GBuffer::startGeometryPass()
{
	dbg_++;

	fbo.bind(pastry::framebuffer::target::WRITE);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, buffers);

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GBuffer::stopGeometryPass()
{
	if(dbg_ == 50) {
		pastry::texture_save(tex_position, "/tmp/deferred_pos.png");
		pastry::texture_save(tex_normal, "/tmp/deferred_normal.png");
		pastry::texture_save(tex_color, "/tmp/deferred_color.png");
		pastry::texture_save(tex_depth, "/tmp/deferred_depth.png");
	}
}

void GBuffer::update()
{
	if(pastry::fb_has_changed()) {
		pastry::fb_get_dimensions(width_, height_);
		fbo.bind();
		tex_position.bind();
		tex_position.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_normal.bind();
		tex_normal.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_color.bind();
		tex_color.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_final.bind();
		tex_final.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_depth.bind();
		tex_depth.set_image_depth<float>(GL_DEPTH_COMPONENT32F, width_, height_);
		fbo.unbind();
	}
}

void GBuffer::startLightPass()
{
	fbo.bind();
	glDrawBuffer(GL_COLOR_ATTACHMENT3);

	glDisable(GL_CULL_FACE);

	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	pastry::texture_2d::activate_unit(0);
	tex_position.bind();		
	pastry::texture_2d::activate_unit(1);
	tex_normal.bind();
	pastry::texture_2d::activate_unit(2);
	tex_color.bind();

}

void GBuffer::stopLightPass()
{
	glDisable(GL_BLEND);

	if(dbg_ == 50) {
		pastry::texture_save(tex_final, "/tmp/deferred_final.png");
	}
}

void GBuffer::finalPass()
{
	fbo.unbind();
	fbo.bind(pastry::framebuffer::target::READ);
	glReadBuffer(GL_COLOR_ATTACHMENT3);
	glBlitFramebuffer(0, 0, width_, height_,
	                  0, 0, width_, height_, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

}}
