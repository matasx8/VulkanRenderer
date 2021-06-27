#pragma once
#include "Mesh.h"
#include <assimp/scene.h>

class Model
{
public:
	Model();
	Model(const Model&);
	Model(std::vector<Mesh> newMeshList);//find out if i can pass by ref

	size_t getMeshCount();
	Mesh* getMesh(size_t index);
	glm::mat4& getModel();
	int getPipelineIndex() const { return pipelineIndex; }

	void setModel(glm::mat4 newModel);
	void setPipelineIndex(int index) { pipelineIndex = index; }
	// Increment pipelineIndex by one. Used when a pipeline has been thrown out.
	void updatePipelineIndex() { pipelineIndex++; }


	void destroyMeshModel();
// move to some model manager or somethig
	static std::vector<std::string> LoadMaterials(const aiScene* scene);
	static std::vector<Mesh> LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiNode* node, const aiScene* scene, std::vector<int> matToTex);
	static Mesh LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex);
	~Model();

private:
	std::vector<Mesh> meshList;
	glm::mat4 model;
	int pipelineIndex;
};

