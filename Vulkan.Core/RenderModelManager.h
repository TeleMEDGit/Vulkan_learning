#pragma once
#include "Common/Common.h"
#include "Model.h"

namespace Graphics
{
	class RenderModelManager
	{
	public:
		RenderModelManager();
		~RenderModelManager();
		void Initialize();


	private:
		std::vector<RenderModel*> m_models;
		RenderModel* m_defaultModel;
		RenderModel* GetModel(const char* modelName, bool createIfNotFound);

	};


}