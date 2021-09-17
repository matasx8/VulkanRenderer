#include "Model.h"

static size_t s_AllTimeModelCount = 0;

Model::Model()
{
	m_IsDuplicate = false;
	m_IsInstanced = false;
	m_Handle = s_AllTimeModelCount++;
}

Model::Model(std::vector<Mesh>& newMeshList)
{
	meshList = newMeshList;
	m_IsDuplicate = false;
	m_IsInstanced = false;
	m_Handle = s_AllTimeModelCount++;
}

size_t Model::getMeshCount()
{
	return meshList.size();
}

Mesh* Model::getMesh(size_t index)
{
	if (index >= meshList.size())
	{
		throw std::runtime_error("attempted to access invalid mesh index");
	}
	return &meshList[index];
}

size_t Model::GetModelHandle() const
{
	return m_Handle;
}

ModelMatrix& Model::GetModelMatrix()
{
	return m_ModelMatrix;
}

Model Model::Duplicate(bool instanced) const
{
	Model tmp = Model(*this);
	tmp.m_Handle = s_AllTimeModelCount++;
	tmp.m_IsDuplicate = true;
	tmp.m_IsInstanced = instanced;
	return tmp;
}

void Model::CopyInInstanceData(void* dst) const
{
	InstanceData data;
	data.model = m_ModelMatrix;
	memcpy(dst, &data, sizeof(InstanceData));
}

void Model::SetModelMatrix(ModelMatrix&& matrix)
{
	m_ModelMatrix = std::move(matrix);
}

void Model::destroyMeshModel()
{
	if (!m_IsDuplicate)
		for (auto& mesh : meshList)
		{
			mesh.destroyBuffers();
		}
}

std::vector<std::string> Model::LoadMaterials(const aiScene* scene)
{
	//create 1:1 sized list of textures
	std::vector<std::string> textureList(scene->mNumMaterials);

	// go through each material and copy its texture file name (if it exists)
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		// get the material
		aiMaterial* material = scene->mMaterials[i];

		// initialise the texture to empty string (will be replaced if texture exists)
		textureList[i] = "";

		// check for a diffuse texure (standart detail texture)
		if (material->GetTextureCount(aiTextureType_DIFFUSE))// the color of an object when light hits it
		{
			// get path of the texture file
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{//TODO: optimise this one day
				// cut off any directory information already present
				int idx = std::string(path.data).rfind("\\");
				std::string fileName = std::string(path.data).substr(idx + 1);

				textureList[i] = fileName;
			}
		}
	}

	return textureList;
}

std::vector<Mesh> Model::LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiNode* node, const aiScene* scene, std::vector<int> matToTex)
{
	std::vector<Mesh> meshList;

	// go through each mesh at this node and create it, then add it to our meshlist
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		meshList.push_back(LoadMesh(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, scene->mMeshes[node->mMeshes[i]], scene, matToTex));
	}

	// go through each node attached to this node and load it, then append their meshes to this node's mesh list
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		std::vector<Mesh> newList = LoadNode(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, node->mChildren[i], scene, matToTex);
		meshList.insert(meshList.end(), newList.begin(), newList.end());
	}

	return meshList;
}

Mesh Model::LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	//resize vertex list to hold all vertices for mesh
	vertices.resize(mesh->mNumVertices);

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		// set position
		vertices[i].pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		// set tex coords (if they exist)
		if (mesh->mTextureCoords[0])
		{
			vertices[i].tex = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		}
		else
		{
			vertices[i].tex = {0.0f, 0.0f};
		}

		vertices[i].norm = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };//do i need to invert?
		//set color (just use white for now)
		//vertices[i].col = { 1.0f, 1.0f, 1.0f };
	}

	// iterate over indices through faces and copy across
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		// get a face
		aiFace face = mesh->mFaces[i];

		// go through face's indices and add to list
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	//create new mesh with details and return it
	Mesh newMesh = Mesh(newPhysicalDevice, newDevice, transferQueue, transferCommandPool, &vertices, &indices, matToTex[mesh->mMaterialIndex]);
	
	return newMesh;
}
