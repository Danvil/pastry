#include <pastry/deferred/DeferredRenderer.hpp>

namespace pastry {
namespace deferred {

DeferredRenderer::DeferredRenderer()
{ }

void DeferredRenderer::setCamera(const std::shared_ptr<pastry::deferred::Camera>& camera)
{ camera_ = camera; }

void DeferredRenderer::add(const std::shared_ptr<pastry::deferred::Geometry>& mesh)
{ geometry_.push_back(mesh); }

void DeferredRenderer::add(const std::shared_ptr<pastry::deferred::Light>& light)
{ lights_.push_back(light); }

void DeferredRenderer::update(float t, float dt)
{

}

void DeferredRenderer::render()
{
	gbuff_.startGeometryPass();
	for(const auto& v : geometry_) {
		v->render(camera_);
	}
	gbuff_.stopGeometryPass();

	gbuff_.startLightPass();
	for(const auto& v : lights_) {
		v->render(camera_);
	}
	gbuff_.stopLightPass();

	gbuff_.finalPass();
}

}}
