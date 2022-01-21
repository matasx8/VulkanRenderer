#include "Model.h"

static size_t s_AllTimeModelCount = 0;

Model::Model()
{
	m_IsDuplicate = false;
	m_IsHidden = false;
	m_IsInstanced = false;
	m_Handle = s_AllTimeModelCount++;
	m_InstanceCount = 1;
	m_InstanceDataBuffer = nullptr;
}

Model::Model(std::vector<Mesh>& newMeshList, bool isInstanced)
{
	meshList = newMeshList;
	m_IsDuplicate = false;
	m_IsHidden = false;
	m_IsInstanced = isInstanced;
	m_Handle = s_AllTimeModelCount++;
	m_InstanceCount = 1;
	m_InstanceDataBuffer = nullptr;
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

void Model::AddInstances(int numInstances)
{
	if (m_IsInstanced && m_InstanceDataBuffer != nullptr)
	{
		m_InstanceDataBuffer->AddInstances(numInstances);
	}
	else
	{
		m_InstanceDataBuffer = new InstanceDataBuffer();
		m_InstanceDataBuffer->Create(GetGraphicsDevice());
		m_InstanceDataBuffer->AddInstances(numInstances + 1);	// + 1 to account for original instance
																// so numInstances + original
	}
	m_InstanceCount += numInstances;
}

void Model::CopyInInstanceData(void* dst) const
{
	InstanceData data;
	data.model = m_ModelMatrix;
	auto ss = sizeof(m_ModelMatrix);
	auto s = sizeof(InstanceData);
	memcpy(dst, &data, sizeof(InstanceData));
}


void Model::SetModelMatrix(const ModelMatrix& matrix)
{
	m_ModelMatrix = matrix;
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
