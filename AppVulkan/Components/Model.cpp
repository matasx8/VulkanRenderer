#include "Model.h"
#include<glm/gtx/matrix_decompose.hpp>

static size_t s_AllTimeModelCount = 0;

Model::Model()
	: m_IsHidden(false), m_IsDuplicate(false), m_IsInstanced(false), m_InstanceCount(0), m_InstanceDataBuffer(nullptr),
	m_Handle(s_AllTimeModelCount++)
{
}

Model::Model(bool isInstanced)
{
	m_IsDuplicate = false;
	m_IsHidden = false;
	m_IsInstanced = isInstanced;
	m_Handle = s_AllTimeModelCount++;
	m_InstanceCount = 1;
	m_InstanceDataBuffer = nullptr;
}

size_t Model::GetModelHandle() const
{
	return m_Handle;
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
	// deprecated
	/*
	InstanceData data;
	data.model = m_ModelMatrix;
	auto ss = sizeof(m_ModelMatrix);
	auto s = sizeof(InstanceData);
	memcpy(dst, &data, sizeof(InstanceData));*/
}


void Model::MoveLocal(const glm::vec3& vector)
{
	m_ModelMatrix = glm::translate(m_ModelMatrix, vector);
}

void Model::RotateLocal(float angle, const glm::vec3& axis)
{
	m_ModelMatrix = glm::rotate(m_ModelMatrix, angle, axis);
}

