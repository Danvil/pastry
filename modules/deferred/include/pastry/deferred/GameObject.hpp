#pragma once

#include <pastry/deferred/Forward.hpp>
#include <memory>
#include <vector>

namespace pastry {
namespace deferred {

class GameObject
: public std::enable_shared_from_this<GameObject>
{
public:
	GameObject();

	void attach(const std::shared_ptr<Component>& c);
	void detach(const std::shared_ptr<Component>& c);

	std::shared_ptr<Scene> scene;

	std::shared_ptr<Camera> camera;
	std::shared_ptr<SkyBox> skybox;
	std::shared_ptr<Light> light;
	std::shared_ptr<Geometry> geometry;

	std::vector<std::shared_ptr<Component>> components;

};

std::shared_ptr<GameObject> FactorGameObject();

std::shared_ptr<GameObject> FactorCamera();

std::shared_ptr<GameObject> FactorSkyBox();

std::shared_ptr<GameObject> FactorEnvironmentLight();
std::shared_ptr<GameObject> FactorPointLight();

std::shared_ptr<GameObject> FactorGeometry();

}}
