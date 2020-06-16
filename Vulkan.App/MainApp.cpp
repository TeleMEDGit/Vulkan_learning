 
#include "../Vulkan.Core/AppCore.h"


using namespace AppCore;




class VulkanViewer : public ICoreApp
{
public:
	VulkanViewer() {}

	virtual void Startup(void) override;
	virtual void Cleanup(void) override;

	virtual void Update(float deltaT) override;
	virtual void RenderScene(void) override;

private:


};

//Note
//This 'CREATE_APPLICATION' requires C/C++/Language/Conformance mode set to 'No'

CREATE_APPLICATION(VulkanViewer)

void VulkanViewer::Startup()
{

}

void VulkanViewer::Cleanup()
{
}

void VulkanViewer::Update(float deltaT)
{
}

void VulkanViewer::RenderScene()
{
	

}