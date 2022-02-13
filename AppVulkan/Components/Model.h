#pragma once

#include "Mesh.h"
#include "InstanceDataBuffer.h"
#include <assimp/scene.h>

// unique identifier for a Model
typedef unsigned int ModelHandle;
typedef glm::mat4x4 ModelMatrix;

class Model
{
public:
	Model();
	//Model(const Model& copyFrom);
	Model(std::vector<Mesh>& newMeshList);
#ifdef _DEBUG
	Model(std::vector<Mesh>& newMeshList, const char* name);
#endif
	Model(std::vector<Mesh>& newMeshList, bool isInstanced = false);

	size_t GetMeshCount() const;
	const Mesh& GetMesh(size_t index) const;
	size_t GetModelHandle() const;
	const glm::mat4x4& GetModelMatrix() const;
	int GetInstanceCount() const { return m_InstanceCount; }
	VkBuffer GetInstanceData() const { return m_InstanceDataBuffer->GetInstanceData(); }
	InstanceDataBuffer* GetInstanceDataBuffer() { return m_InstanceDataBuffer; }
	bool IsHidden() const { return m_IsHidden; }
	glm::vec4 GetColorCode() const;

	Model Duplicate(bool instanced = false) const;
	void AddInstances(int numInstances);
	// make sure dst is at least the size of InstanceData struct
	void CopyInInstanceData(void* dst) const;
	template<typename Tfunc, typename... Targs>
	void ApplyFunc(Tfunc func, Targs&... args);

	void MoveLocal(const glm::vec3& vector);
	void RotateLocal(float angle, const glm::vec3& axis);

	void SetModelMatrix(const ModelMatrix& matrix);
	void SetIsHidden(bool isHidden) { m_IsHidden = isHidden; }
	// move this to model manager, when i move meshes out of model class
	void SetMaterialForAllMeshes(uint32_t materialID);


	bool IsInstanced() const { return m_IsInstanced; }

	void destroyMeshModel();
// move to some model manager or somethig
	// moved, now delete when confirmed working
	static std::vector<std::string> LoadMaterials(const aiScene* scene);
	static std::vector<Mesh> LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiNode* node, const aiScene* scene, std::vector<int> matToTex);
	static Mesh LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex);

private:
	
	// Duplicates won't have their own memory and will adress to the memory of the original
	bool m_IsHidden;
	bool m_IsDuplicate;

	bool m_IsInstanced;
	int m_InstanceCount;
	InstanceDataBuffer* m_InstanceDataBuffer;

	ModelHandle m_Handle;
	std::vector<Mesh> meshList;
	ModelMatrix m_ModelMatrix;
#ifdef _DEBUG
	//std::string m_Name;
#endif
};

template<typename Tfunc, typename... Targs>
void Model::ApplyFunc(Tfunc func, Targs&... args)
{
	if (m_InstanceDataBuffer)
		m_InstanceDataBuffer->ApplyFunctionForEach(func, args...);
	else
		throw std::runtime_error("This model is not instanced! Can't apply function to instances.");
}
