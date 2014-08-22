#include <pastry/deferred/GBuffer.hpp>
#include <pastry/pastry.hpp>

namespace pastry {
namespace deferred {

GBuffer::GBuffer()
{
	dbg_ = -1;

	pastry::fb_get_dimensions(width_, height_);
	fbo.bind();

	tex_final.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_final.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT0, tex_final);

	tex_position.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_position.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT1, tex_position);

	tex_normal.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_normal.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT2, tex_normal);

	tex_color.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_color.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT3, tex_color);

	tex_material.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_material.set_image<float, 3>(GL_RGB32F, width_, height_);
	fbo.attach(GL_COLOR_ATTACHMENT4, tex_material);

	tex_depth.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
	tex_depth.set_image_depth_stencil(GL_DEPTH24_STENCIL8, width_, height_);
	fbo.attach(GL_DEPTH_STENCIL_ATTACHMENT, tex_depth);

	fbo.unbind();
}

void GBuffer::update()
{
	if(pastry::fb_has_changed()) {
		pastry::fb_get_dimensions(width_, height_);
		fbo.bind();
		tex_final.bind();
		tex_final.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_position.bind();
		tex_position.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_normal.bind();
		tex_normal.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_color.bind();
		tex_color.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_material.bind();
		tex_material.set_image<float, 3>(GL_RGB32F, width_, height_);
		tex_depth.bind();
		tex_depth.set_image_depth<float>(GL_DEPTH24_STENCIL8, width_, height_);
		fbo.unbind();
	}
}

void GBuffer::prePass()
{
	dbg_++;

	// clear all
	fbo.bind();
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, buffers);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	fbo.unbind();

	glDisable(GL_STENCIL_TEST);
}

void GBuffer::startGeometryPass()
{
	// use g-buffers for rendering
	fbo.bind(pastry::framebuffer::target::WRITE);
	GLenum buffers[] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(4, buffers);

	// set render settings
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// setup stencil buffer
	// all pixel which pass the depth test are marked with 255
	// pixel which are not rendered are 0 (due to glClearStencil)
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 255, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void GBuffer::stopGeometryPass()
{
	glDisable(GL_STENCIL_TEST);

	if(dbg_ == 0) {
		std::cout << "Storing GBuffer to files (1)" << std::endl;
		pastry::texture_save(tex_position, "/tmp/deferred_pos.png");
		pastry::texture_save(tex_normal, "/tmp/deferred_normal.png");
		pastry::texture_save(tex_color, "/tmp/deferred_color.png");
		pastry::texture_save(tex_material, "/tmp/deferred_material.png");
		pastry::texture_save(tex_depth, "/tmp/deferred_depth.png");
	}
}

void GBuffer::startForwardPass()
{
	// use final buffer for rendering
	fbo.bind(pastry::framebuffer::target::WRITE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// set render settings
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void GBuffer::stopForwardPass()
{
	if(dbg_ == 0) {
		std::cout << "Storing GBuffer to files (2)" << std::endl;
		pastry::texture_save(tex_final, "/tmp/deferred_final_pre.png");
	}
}

void GBuffer::startLightPass()
{
	// use final buffer for rendering
	fbo.bind(pastry::framebuffer::target::WRITE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	// do net change depth buffer
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	// use stencil buffer to mask fragments
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_LEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// enable blending to add light
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// bind textures
	pastry::texture_2d::activate_unit(0);
	tex_position.bind();		
	pastry::texture_2d::activate_unit(1);
	tex_normal.bind();
	pastry::texture_2d::activate_unit(2);
	tex_color.bind();
	pastry::texture_2d::activate_unit(3);
	tex_material.bind();
}

void GBuffer::stopLightPass()
{
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_TRUE);

	if(dbg_ == 0) {
		std::cout << "Storing GBuffer to files (3)" << std::endl;
		pastry::texture_save(tex_final, "/tmp/deferred_final.png");
	}
}

void GBuffer::finalPass()
{
	// bind normal framebuffer for write and final buffer for read
	fbo.unbind();
	fbo.bind(pastry::framebuffer::target::READ);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	// copy contents
	glBlitFramebuffer(0, 0, width_, height_,
	                  0, 0, width_, height_, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

}}
