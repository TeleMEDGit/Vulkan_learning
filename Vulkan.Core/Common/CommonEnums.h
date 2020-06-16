#pragma once
namespace Graphics
{

#ifndef BIT
#define BIT( num )				( 1ULL << ( num ) )
#endif

	//extern Bootup g_vBootup;
	static const int MAX_RENDER_CROPS = 8;
	// this is the inital allocation for max number of drawsurfs
	// in a given view, but it will automatically grow if needed
	const int INITIAL_DRAWSURFS = 2048;

	static const unsigned int FRAME_ALLOC_ALIGNMENT = 128;
	static const unsigned int MAX_FRAME_MEMORY = 64 * 1024 * 1024;	// larger so that we can noclip on PC for dev purposes

	enum vulkanMemoryUsage_t {
		VULKAN_MEMORY_USAGE_UNKNOWN,
		VULKAN_MEMORY_USAGE_GPU_ONLY,
		VULKAN_MEMORY_USAGE_CPU_ONLY,
		VULKAN_MEMORY_USAGE_CPU_TO_GPU,
		VULKAN_MEMORY_USAGE_GPU_TO_CPU,
		VULKAN_MEMORY_USAGES,
	};

	enum vulkanAllocationType_t {
		VULKAN_ALLOCATION_TYPE_FREE,
		VULKAN_ALLOCATION_TYPE_BUFFER,
		VULKAN_ALLOCATION_TYPE_IMAGE,
		VULKAN_ALLOCATION_TYPE_IMAGE_LINEAR,
		VULKAN_ALLOCATION_TYPE_IMAGE_OPTIMAL,
		VULKAN_ALLOCATION_TYPES,
	};

	

	enum graphicsVendor_t {
		VENDOR_NVIDIA,
		VENDOR_AMD,
		VENDOR_INTEL
	};

	enum frameAllocType_t {
		FRAME_ALLOC_VIEW_DEF,
		FRAME_ALLOC_VIEW_ENTITY,
		FRAME_ALLOC_VIEW_LIGHT,
		FRAME_ALLOC_SURFACE_TRIANGLES,
		FRAME_ALLOC_DRAW_SURFACE,
		FRAME_ALLOC_INTERACTION_STATE,
		FRAME_ALLOC_SHADOW_ONLY_ENTITY,
		FRAME_ALLOC_SHADOW_VOLUME_PARMS,
		FRAME_ALLOC_SHADER_REGISTER,
		FRAME_ALLOC_DRAW_SURFACE_POINTER,
		FRAME_ALLOC_DRAW_COMMAND,
		FRAME_ALLOC_UNKNOWN,
		FRAME_ALLOC_MAX
	};

	static const int MAX_TAGS = 256;
	enum MemoryTag
	{
		TAG_RENDER,
		TAG_IDLIB_HASH,
		TAG_CRAP,
		TAG_IMAGE,
		TAG_LIST_IMAGE,
		TAG_TEMP,
		TAG_THREAD,
		TAG_SRFTRIS,
		TAG_TRI_VERTS,
		TAG_TRI_INDEXES,
		TAG_RENDER_TOOLS,
		TAG_DECL,
		TAG_RENDER_STATIC,
		TAG_MODEL,
		TAG_RENDER_ENTITY,
	};



	enum TextureType {
		TEXTURE_TYPE_DISABLED,
		TEXTURE_TYPE_2D,
		TEXTURE_TYPE_CUBIC
	};

	/*
	================================================
	The internal *Texture Format Types*, ::textureFormat_t, are:
	================================================
	*/
	enum TextureFormat {
		TEXTURE_FORMAT_NONE,

		//------------------------
		// Standard color image formats
		//------------------------

		TEXTURE_FORMAT_RGBA8,			// 32 bpp
		TEXTURE_FORMAT_XRGB8,			// 32 bpp

		//------------------------
		// Alpha channel only
		//------------------------

		// Alpha ends up being the same as L8A8 in our current implementation, because straight 
		// alpha gives 0 for color, but we want 1.
		TEXTURE_FORMAT_ALPHA,

		//------------------------
		// Luminance replicates the value across RGB with a constant A of 255
		// Intensity replicates the value across RGBA
		//------------------------

		TEXTURE_FORMAT_L8A8,			// 16 bpp
		TEXTURE_FORMAT_LUM8,			//  8 bpp
		TEXTURE_FORMAT_INT8,			//  8 bpp

		//------------------------
		// Compressed texture formats
		//------------------------

		TEXTURE_FORMAT_DXT1,			// 4 bpp
		TEXTURE_FORMAT_DXT5,			// 8 bpp

		//------------------------
		// Depth buffer formats
		//------------------------

		TEXTURE_FORMAT_DEPTH,			// 24 bpp

		//------------------------
		//
		//------------------------

		TEXTURE_FORMAT_X16,			// 16 bpp
		TEXTURE_FORMAT_Y16_X16,		// 32 bpp
		TEXTURE_FORMAT_RGB565,			// 16 bpp
	};


	enum TextureSamples {
		SAMPLE_1 = BIT(0),
		SAMPLE_2 = BIT(1),
		SAMPLE_4 = BIT(2),
		SAMPLE_8 = BIT(3),
		SAMPLE_16 = BIT(4)
	};
	/*
	================================================
	DXT5 color formats
	================================================
	*/
	enum TextureColor {
		COLOR_FORMAT_DEFAULT,			// RGBA
		COLOR_FORMAT_NORMAL_DXT5,		// XY format and use the fast DXT5 compressor
		COLOR_FORMAT_YCOCG_DXT5,	    // convert RGBA to CoCg_Y format
		COLOR_FORMAT_GREEN_ALPHA	    // Copy the alpha channel to green
	};


	typedef enum {
		CUBE_FILE_2D,			// not a cube map
		CUBE_FILE_NATIVE,		// _px, _nx, _py, etc, directly sent to GL
		CUBE_FILE_CAMERA		// _forward, _back, etc, rotated and flipped as needed before sending to GL
	} CubeFiles;

	typedef enum {
		TD_SPECULAR,			// may be compressed, and always zeros the alpha channel
		TD_DIFFUSE,				// may be compressed
		TD_DEFAULT,				// generic RGBA texture (particles, etc...)
		TD_BUMP,				// may be compressed with 8 bit lookup
		TD_FONT,				// Font image
		TD_LIGHT,				// Light image
		TD_LOOKUP_TABLE_MONO,	// Mono lookup table (including alpha)
		TD_LOOKUP_TABLE_ALPHA,	// Alpha lookup table with a white color channel
		TD_LOOKUP_TABLE_RGB1,	// RGB lookup table with a solid white alpha
		TD_LOOKUP_TABLE_RGBA,	// RGBA lookup table
		TD_COVERAGE,			// coverage map for fill depth pass when YCoCG is used
		TD_DEPTH,				// depth buffer copy for motion blur
	} TextureUsage;


	typedef enum {
		TEXTURE_FILTER_LINEAR,
		TEXTURE_FILTER_NEAREST,
		TEXTURE_FILTER_DEFAULT				// use the user-specified r_textureFilter
	} TextureFilter;

	typedef enum {
		TEXTURE_REPEAT_REPEAT,
		TEXTURE_REPEAT_CLAMP,
		TEXTURE_REPEAT_CLAMP_TO_ZERO,		// guarantee 0,0,0,255 edge for projected textures
		TEXTURE_REPEAT_CLAMP_TO_ZERO_ALPHA	// guarantee 0 alpha edge for projected textures
	} TextureRepeat;

	
	enum RenderOption {
		RC_NOP,
		RC_DRAW_VIEW,
		RC_COPY_RENDER,
	};


}
