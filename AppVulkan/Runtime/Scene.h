#pragma once
#include <string>
#include <vector>
#include "Model.h"
#include "vulkan.h"
#include "Material.h"
#include "Pipeline.h"
#include "Containers/RenderPassManager.h"
#include "Utilities.h"

class Camera;
class Light;

class Scene
{
public:
	Scene() {};
	Scene(VkExtent2D extent);
	
	void addLight();

	void* getViewProjectionPtr() const { return (void*)(&viewProjection); }
	ViewProjectionMatrix& GetViewProjectionMatrix() { return viewProjection; }


	// Updates components (eg. Lights, Camera..)
	void updateScene(size_t index);

	void onFrameEnded();
	void CleanUp(VkDevice logicalDevice);

	// -- temporary Input
	void cameraKeyControl(bool* keys, float dt);
	void cameraMouseControl(float xChange, float yChange);
	// -- temporary Input

private:
	
	friend class VulkanRenderer;

	ViewProjectionMatrix viewProjection;

	Camera camera;
	std::vector<Light> Lights;

};

