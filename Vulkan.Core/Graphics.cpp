#include "Graphics.h"
#include "RenderSystem.h"
#include "SystemTime.h"


namespace AppCore
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	extern HWND g_hWnd;			//Window hook to 3D API
	extern HINSTANCE g_hInst;   //Instance hook to 3D API
#else
	extern Platform::Agile<Windows::UI::Core::CoreWindow>  g_window;
#endif
}

namespace
{
	float s_FrameTime = 0.0f;
	uint64_t s_FrameIndex = 0;
	int64_t s_FrameStartTick = 0;
	bool s_EnableVSync = true;
	bool s_LimitTo30Hz = false;
	bool s_DropRandomFrames = false;

}


namespace Graphics
{
	int isFirst = 0;
	uint32_t g_Width = 1280;
	uint32_t g_Height = 720;
		
	RenderSystem  render_system;

	FrameTimeTick m_frameTiming;

	DeviceContext m_device_context;

	void Initialize()
	{
		render_system.Initialize();
		


	}

	void Terminate()
	{



	}




	void Resize(uint32_t width, uint32_t height)
	{
		g_Width = width;
		g_Height = height;

		if (isFirst > 0)
			render_system.Restart();

		isFirst = 1;

		
	}

	void Shutdown(void)
	{


	}

	void Present()
	{
		SetCurrentTick();

		render_system.SwapAndRenderCommandBuffers(&m_frameTiming);



		//set up view and model data and call this line

		RenderModel* renderModel = nullptr;  //this should be setting somewhere else and should have model data also

		renderView* view = new (TAG_MODEL) renderView;
		memset(view, 0, sizeof(view));

		view->viewID = 0;
		view->fov_x = 67.30f;
		view->fov_y = 41.11f;

		view->vieworg = Math::Vector3(-3598.44f, -3625.35f, 247.46f);
		view->vieworg_weapon = Math::Vector3(0.0f, 0.0f, 0.0f);
		view->viewaxis = Math::Matrix3(
			Math::Vector3(0.77f, 0.43f, -0.45f), 
			Math::Vector3(0.49f, 0.87f, 7.45f),
			Math::Vector3(0.39f, -0.22f, 0.89f));
		view->cramZNear = false;
		view->flipProjection = false;
		view->forceUpdate = false;
		view->shaderParms;



		render_system.RenderScene(renderModel, view);

		//TDO fix frame time logic and comapre what evere 
		//Utility::Printf("System Tick %d", s_FrameTime);

		
	}

	void SetCurrentTick()
	{
		UINT PresentInterval = std::min(4, (int)std::round(s_FrameTime * 60.0f));

		int64_t CurrentTick = SystemTime::GetCurrentTick();


		if (s_EnableVSync)
		{
			// With VSync enabled, the time step between frames becomes a multiple of 16.666 ms.  We need
			// to add logic to vary between 1 and 2 (or 3 fields).  This delta time also determines how
			// long the previous frame should be displayed (i.e. the present interval.)
			s_FrameTime = (s_LimitTo30Hz ? 2.0f : 1.0f) / 60.0f;
			if (s_DropRandomFrames)
			{
				if (std::rand() % 50 == 0)
					s_FrameTime += (1.0f / 60.0f);
			}

		}
		else
		{
			s_FrameTime = (float)SystemTime::TimeBetweenTicks(s_FrameStartTick, CurrentTick);
		}


		s_FrameStartTick = CurrentTick;

		++s_FrameIndex;

	}

	uint64_t GetFrameCount()
	{
		return s_FrameIndex;
	}

	float GetFrameTime()
	{
		return s_FrameTime;
	}

	float GetFrameRate()
	{
		return s_FrameTime == 0.0f ? 0.0f : 1.0f / s_FrameTime;
	}
	
}
