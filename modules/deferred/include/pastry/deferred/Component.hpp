#pragma once

#include <pastry/deferred/Forward.hpp>
#include <pastry/deferred/GameObject.hpp>
#include <pastry/deferred/Scene.hpp>
#include <memory>

namespace pastry {
namespace deferred {

class Component
{
public:
	virtual ~Component() {}

	std::shared_ptr<GameObject> gameObject;

	const std::shared_ptr<Camera>& mainCamera() const
	{ return gameObject->scene->mainCamera(); }

	const std::shared_ptr<SkyBox>& mainSkybox() const
	{ return gameObject->scene->mainSkybox(); }

	const std::shared_ptr<Camera>& camera() const
	{ return gameObject->camera; }

	const std::shared_ptr<SkyBox>& skybox() const
	{ return gameObject->skybox; }

	const std::shared_ptr<Light>& light() const
	{ return gameObject->light; }

	const std::shared_ptr<Geometry>& geometry() const
	{ return gameObject->geometry; }
};

}}
