#pragma once

#include <pastry/deferred/Forward.hpp>
#include <pastry/deferred/GameObject.hpp>
#include <vector>
#include <memory>

namespace pastry {
namespace deferred {

class Scene
: public std::enable_shared_from_this<Scene>
{
public:
	Scene();

	void setMainCamera(const std::shared_ptr<GameObject>& go);

	void setMainSkybox(const std::shared_ptr<GameObject>& go);

	const std::shared_ptr<Camera>& mainCamera()
	{ return main_camera_; }

	const std::shared_ptr<SkyBox>& mainSkybox()
	{ return main_skybox_; }

	void add(const std::shared_ptr<GameObject>& go);
	void remove(const std::shared_ptr<GameObject>& go);

	template<typename T>
	void add(const std::shared_ptr<T>& c)
	{
		auto geometry = std::dynamic_pointer_cast<Geometry>(c);
		if(geometry) add(geometry);
		auto light = std::dynamic_pointer_cast<Light>(c);
		if(light) add(light);
	}

	template<typename T>
	void remove(const std::shared_ptr<T>& c)
	{
		auto geometry = std::dynamic_pointer_cast<Geometry>(c);
		if(geometry) remove(geometry);
		auto light = std::dynamic_pointer_cast<Light>(c);
		if(light) remove(light);
	}

	void add(const std::shared_ptr<Geometry>& geometry);
	void add(const std::shared_ptr<Light>& light);

	void remove(const std::shared_ptr<Geometry>& geometry);
	void remove(const std::shared_ptr<Light>& light);

	const std::vector<std::shared_ptr<GameObject>>& gameObjects() const
	{ return game_objects_; }

	const std::vector<std::shared_ptr<Geometry>>& geometry() const
	{ return geometry_; }

	const std::vector<std::shared_ptr<Light>>& lights() const
	{ return lights_; }

private:
	std::shared_ptr<Camera> main_camera_;
	std::shared_ptr<SkyBox> main_skybox_;
	std::vector<std::shared_ptr<GameObject>> game_objects_;
	std::vector<std::shared_ptr<Geometry>> geometry_;
	std::vector<std::shared_ptr<Light>> lights_;
};

}}
