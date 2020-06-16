#include "RenderModelManager.h"

namespace Graphics
{
	

	RenderModelManager::RenderModelManager()
	{
	}

	RenderModelManager::~RenderModelManager()
	{
	}

	void RenderModelManager::Initialize()
	{
		RenderModel* model = new (TAG_MODEL) RenderModel;
		model->InitEmpty("_DEFAULT");
		model->MakeDefaultModel();
		m_defaultModel = model;


	}

	RenderModel* RenderModelManager::GetModel(const char* modelName, bool createIfNotFound)
	{
		//RenderModel* model = m_models[0];

		RenderModel* model = nullptr;

		if (true /*static*/)
			model = new (TAG_MODEL)RenderModel;


		return model;
	}

}