#include <pastry/deferred/Scene.hpp>
#include <pastry/deferred/Tools.hpp>

namespace pastry {
namespace deferred {


Scene::Scene()
{

}

void Scene::setMainCamera(const std::shared_ptr<GameObject>& go)
{
	remove(go);
	main_camera_ = go->camera;
	add(go);
}

void Scene::setMainSkybox(const std::shared_ptr<GameObject>& go)
{
	remove(go);
	main_skybox_ = go->skybox;
	add(go);
}


void Scene::add(const std::shared_ptr<GameObject>& go)
{
	if(!go) return;
	UniqueAdd(game_objects_, go);
	if(go->geometry) {
		add(go->geometry);
	}
	if(go->light) {
		add(go->light);
	}
	go->scene = shared_from_this();
}

void Scene::remove(const std::shared_ptr<GameObject>& go)
{
	if(!go) return;
	UniqueRemove(game_objects_, go);
	if(go->geometry) {
		remove(go->geometry);
	}
	if(go->light) {
		remove(go->light);
	}
	go->scene = nullptr;
}

void Scene::add(const std::shared_ptr<pastry::deferred::Geometry>& geometry)
{
	UniqueAdd(geometry_, geometry);
}

void Scene::add(const std::shared_ptr<pastry::deferred::Light>& light)
{
	UniqueAdd(lights_, light);
}

void Scene::remove(const std::shared_ptr<Geometry>& geometry)
{
	UniqueRemove(geometry_, geometry);
}

void Scene::remove(const std::shared_ptr<Light>& light)
{
	UniqueRemove(lights_, light);
}

}}
