
#include <pastry/deferred/DeferredRenderer.hpp>
#include <pastry/deferred/Camera.hpp>
#include <pastry/deferred/Geometry.hpp>
#include <pastry/deferred/Light.hpp>
#include <pastry/deferred/SkyBox.hpp>
#include <pastry/deferred/Scene.hpp>
#include <pastry/obj.hpp>
#include <pastry/pastry.hpp>
#include <pastry/gl.hpp>
#include <iostream>
#include <string>
#include <cmath>

Eigen::Vector3f HSL(float hue, float sat, float light)
{
	float c = (1.0f - std::abs(2.0f*light - 1.0f)) * sat;
	while(hue < 0) hue += 1;
	while(hue >= 1) hue -= 1;
	float h = hue * 6.0f;
	float x = c * (1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f));
	float r,g,b;
	     if(h < 1) { r = c; g = x; b = 0; }
	else if(h < 2) { r = x; g = c; b = 0; }
	else if(h < 3) { r = 0; g = c; b = x; }
	else if(h < 4) { r = 0; g = x; b = c; }
	else if(h < 5) { r = x; g = 0; b = c; }
	else           { r = c; g = 0; b = x; }
	float m = light - 0.5f * c;
	return {r+m, g+m, b+m};
}

int main(void)
{
	std::cout << "Starting engine" << std::endl;

	pastry::initialize();

	auto dr = std::make_shared<pastry::deferred::DeferredRenderer>();
	pastry::scene_add(dr);

	// scene (need this first!)
	auto scene = std::make_shared<pastry::deferred::Scene>();
	dr->setScene(scene);

	// skybox
	{
		auto go = pastry::deferred::FactorSkyBox();
		go->skybox->setTexture("assets/stormydays_cm.jpg");
		go->skybox->setGeometry("assets/sphere_4.obj");
		scene->setMainSkybox(go);
	}

	// camera
	{
		auto go = pastry::deferred::FactorCamera();
		go->camera->setProjection(90.0f, 1.0f, 100.0f);
		go->camera->setView({4,14,6},{2,3,0},{0,0,-1});
		scene->setMainCamera(go);
	}

	constexpr float SPACE = 3.0f;

	// geometry
	{
		constexpr int R = 3;
		for(int x=-R; x<=+R; x++) {
			for(int y=-R; y<=+R; y++) {
				auto go = pastry::deferred::FactorGeometry();
				go->geometry->load("assets/suzanne.obj");
				Eigen::Affine3f pose = Eigen::Translation3f(Eigen::Vector3f(SPACE*x,SPACE*y,0))
					* Eigen::AngleAxisf(0.0f,Eigen::Vector3f{0,0,1});
				go->geometry->setPose(pose.matrix());
				float p = static_cast<float>(x+R)/static_cast<float>(2*R);
				go->geometry->setRoughness(0.2f + 0.8f*p);
				scene->add(go);				
			}
		}
	}

	// lights
	// {
	// 	auto light = std::make_shared<pastry::deferred::PointLight>();
	// 	light->setLightPosition({+3,3,4});
	// 	light->setLightColor(20.0f*Eigen::Vector3f{1,1,1});
	// 	scene->add(light);
	// }
	// {
	// 	auto light = std::make_shared<pastry::deferred::PointLight>();
	// 	light->setLightPosition({-4,1,4});
	// 	light->setLightColor(20.0f*Eigen::Vector3f{1,0.5,0.5});
	// 	scene->add(light);
	// }
	{
		auto go = pastry::deferred::FactorEnvironmentLight();
		scene->add(go);
	}
	{
		constexpr int R = 3;
		for(int x=-R; x<=+R; x++) {
			for(int y=-R; y<=+R; y++) {
				auto go = pastry::deferred::FactorPointLight();
				((pastry::deferred::PointLight*)(go->light.get()))->setLightPosition({SPACE*x,SPACE*y,1.5});
				((pastry::deferred::PointLight*)(go->light.get()))->setLightColor(35.0f*HSL(std::atan2(y,x)/6.2831853f,0.5f,0.5f));
				((pastry::deferred::PointLight*)(go->light.get()))->setLightFalloff(0.05f);
				scene->add(go);
			}
		}
	}

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}
