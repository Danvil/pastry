#include <pastry/deferred/Scene.hpp>
#include <algorithm>

namespace pastry {
namespace deferred {

template<typename T>
void add_impl(std::vector<std::shared_ptr<T>>& v, const std::shared_ptr<T>& x)
{
	if(!x) return;
	auto it = std::find(v.begin(), v.end(), x);
	if(it == v.end()) {
		v.push_back(x);
	}
}

template<typename T>
void remove_impl(std::vector<std::shared_ptr<T>>& v, const std::shared_ptr<T>& x)
{
	if(!x) return;
	auto it = std::find(v.begin(), v.end(), x);
	if(it != v.end()) {
		v.erase(it);
	}
}


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
	add_impl(game_objects_, go);
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
	remove_impl(game_objects_, go);
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
	add_impl(geometry_, geometry);
}

void Scene::add(const std::shared_ptr<pastry::deferred::Light>& light)
{
	add_impl(lights_, light);
}

void Scene::remove(const std::shared_ptr<Geometry>& geometry)
{
	remove_impl(geometry_, geometry);
}

void Scene::remove(const std::shared_ptr<Light>& light)
{
	remove_impl(lights_, light);
}

}}
