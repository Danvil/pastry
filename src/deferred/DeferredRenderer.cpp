#include <pastry/deferred/DeferredRenderer.hpp>
#include <pastry/deferred/Scene.hpp>
#include <pastry/deferred/Camera.hpp>
#include <pastry/deferred/SkyBox.hpp>
#include <pastry/deferred/Geometry.hpp>
#include <pastry/deferred/Light.hpp>
#include <pastry/obj.hpp>
#include <pastry/pastry.hpp>

namespace pastry {
namespace deferred {

DeferredRenderer::DeferredRenderer()
{}

void DeferredRenderer::setScene(const std::shared_ptr<Scene>& scene)
{
	scene_ = scene;
}

void DeferredRenderer::update(float t, float dt)
{
	scene_->mainCamera()->update();
	gbuff_.update();
}

void DeferredRenderer::render()
{
	gbuff_.prePass();

	gbuff_.startGeometryPass();
	for(const auto& v : scene_->geometry()) {
		v->render();
	}
	gbuff_.stopGeometryPass();

	gbuff_.startForwardPass();
	scene_->mainSkybox()->render();
	gbuff_.stopForwardPass();

	gbuff_.startLightPass();
	for(const auto& v : scene_->lights()) {
		v->render();
	}
	gbuff_.stopLightPass();

	gbuff_.finalPass();
}

}}
