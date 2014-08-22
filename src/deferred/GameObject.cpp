#include <pastry/deferred/GameObject.hpp>
#include <pastry/deferred/Camera.hpp>
#include <pastry/deferred/Geometry.hpp>
#include <pastry/deferred/Light.hpp>
#include <pastry/deferred/Scene.hpp>
#include <pastry/deferred/Script.hpp>
#include <pastry/deferred/SkyBox.hpp>
#include <exception>

namespace pastry {
namespace deferred {

template<typename C>
bool IsOfType(const std::shared_ptr<Component>& c)
{
	return std::dynamic_pointer_cast<C>(c).get() != nullptr;
}

GameObject::GameObject()
{

}

#define ITERATE_COMPONENTS(M) \
	M(Camera, camera) \
	M(SkyBox, skybox) \
	M(Light, light) \
	M(Geometry, geometry) \
	M(Script, script)

void GameObject::attach(const std::shared_ptr<Component>& c)
{
	// general
	c->gameObject = shared_from_this();
	// special
	#define PDGO_ATTACH(T,N) if(IsOfType<T>(c)) { this->N = std::dynamic_pointer_cast<T>(c); return; }
	ITERATE_COMPONENTS(PDGO_ATTACH)
	#undef PDGO_ATTACH
	// active scene objects
	if(scene) {
		scene->add(c);
	}
	// special else:
	throw std::runtime_error("Unknown Component type!");
}

void GameObject::detach(const std::shared_ptr<Component>& c)
{
	if(c->gameObject != shared_from_this()) {
		throw std::runtime_error("Component is not attached to GameObject!");
	}
	c->gameObject = nullptr;
	// special
	#define PDGO_DETACH(T,N) if(IsOfType<T>(c)) { this->N = nullptr; return; }
	ITERATE_COMPONENTS(PDGO_DETACH)
	#undef PDGO_DETACH
	// active scene objects
	if(scene) {
		scene->remove(c);
	}
	// special else:
	throw std::runtime_error("Unknown Component type!");
}

std::shared_ptr<GameObject> FactorGameObject()
{
	auto go = std::make_shared<GameObject>();
	return go;
}

std::shared_ptr<GameObject> FactorCamera()
{
	auto go = FactorGameObject();
	auto c = std::make_shared<Camera>();
	go->attach(c);
	return go;
}

std::shared_ptr<GameObject> FactorSkyBox()
{
	auto go = FactorGameObject();
	auto c = std::make_shared<SkyBox>();
	go->attach(c);
	return go;
}

std::shared_ptr<GameObject> FactorEnvironmentLight()
{
	auto go = FactorGameObject();
	auto c = std::make_shared<EnvironmentLight>();
	go->attach(c);
	return go;
}

std::shared_ptr<GameObject> FactorPointLight()
{
	auto go = FactorGameObject();
	auto c = std::make_shared<PointLight>();
	go->attach(c);
	return go;
}

std::shared_ptr<GameObject> FactorGeometry()
{
	auto go = FactorGameObject();
	auto c = std::make_shared<Geometry>();
	go->attach(c);
	return go;
}

std::shared_ptr<GameObject> FactorScript()
{
	auto go = FactorGameObject();
	auto c = std::make_shared<Script>();
	go->attach(c);
	return go;
}

}}
