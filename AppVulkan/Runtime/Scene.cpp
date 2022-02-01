#include "Scene.h"
#include "Camera.h"
#include "glm/glm.hpp"

Scene::Scene(VkExtent2D extent)
{
    //viewProjection.projection = glm::ortho()
    viewProjection.projection = glm::perspective(glm::radians(180.0f), (float)extent.width / (float)extent.height, 0.1f, 10000.0f);
    viewProjection.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 150.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    viewProjection.projection[1][1] *= -1; // invert
}


void Scene::addLight()
{
    Lights.emplace_back(glm::vec4(0.0f, 100.0f, 0.0f, 0.0f));
}

void Scene::updateScene(size_t index)
{
    viewProjection.view = camera.calculateViewMatrix();
}


void Scene::onFrameEnded()
{

}

void Scene::CleanUp(VkDevice logicalDevice)
{


}

void Scene::cameraKeyControl(bool* keys, float dt)
{
    camera.keyControl(keys, dt);
}

void Scene::cameraMouseControl(float xChange, float yChange)
{
    camera.mouseControl(xChange, yChange);
}


