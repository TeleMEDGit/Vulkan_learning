#include "Image.h"
#include "../External/3DModelFromObjFile.h"
#include "../Graphics.h"
#include "VulkanStagingBuffer.h"

#include "..\RenderSystem.h"
#include "../DXT/DXTCodec.h"



namespace Graphics
{
	
	int							Image::m_garbage_index = 0;
#define	DEFAULT_SIZE 16


#if defined( ID_USE_AMD_ALLOCATOR )

	std::vector< VmaAllocation > Image::m_allocation_garbage[NUM_FRAME_DATA];
#else

	std::vector< VulkanAllocation > Image::m_allocation_garbage[NUM_FRAME_DATA];
#endif
	// we build a canonical token form of the image program here
	static char parseBuffer[MAX_IMAGE_NAME];






	std::vector< VkImage >			Image::m_image_garbage[NUM_FRAME_DATA];
	std::vector< VkImageView >		Image::m_view_garbage[NUM_FRAME_DATA];
	std::vector< VkSampler >		Image::m_sampler_garbage[NUM_FRAME_DATA];


	ImageManager	imageManager;
	ImageManager * global_images = &imageManager;

	FileSystem* file_system;



	static bool R_ParseImageProgram_r(std::string &src, std::vector<byte> & pic, int *width, int *height,
		int64 *timestamps, TextureUsage * usage) {

		int64	timestamp;

		//TDO here where all maps and other image based are loaded

		// load it as an image
		R_LoadImage(src, pic, width, height, &timestamp, true);


		return true;;
	}

	static void R_RGBA8Image(Image *image) {
		const int width = render_system.GetWidth();
		const int height = render_system.GetHeight();
		const int size = width * height * 4;



		byte * data = (byte *)Mem_ClearedAlloc(size, TAG_IMAGE);

		memset(data, 0, sizeof(data));
		image->GenerateImage(
			(byte *)data,
			width, height,
			TEXTURE_FILTER_DEFAULT, TEXTURE_REPEAT_REPEAT, TD_LOOKUP_TABLE_RGBA);

		Mem_Free(data);
	}

	/*
	==================
	idImage::MakeDefault

	the default image will be grey with a white box outline
	to allow you to see the mapping coordinates on a surface
	==================
	*/
	static void DefaultImage(Image *image) {
		int		x, y;
		byte	data[DEFAULT_SIZE][DEFAULT_SIZE][4];

		memset(data, 0, sizeof(data));

		if (true) {
			// grey center
			for (y = 0; y < DEFAULT_SIZE; y++) {
				for (x = 0; x < DEFAULT_SIZE; x++) {
					data[y][x][0] = 32;
					data[y][x][1] = 32;
					data[y][x][2] = 32;
					data[y][x][3] = 255;
				}
			}

			// white border
			for (x = 0; x < DEFAULT_SIZE; x++) {
				data[0][x][0] =
					data[0][x][1] =
					data[0][x][2] =
					data[0][x][3] = 255;

				data[x][0][0] =
					data[x][0][1] =
					data[x][0][2] =
					data[x][0][3] = 255;

				data[DEFAULT_SIZE - 1][x][0] =
					data[DEFAULT_SIZE - 1][x][1] =
					data[DEFAULT_SIZE - 1][x][2] =
					data[DEFAULT_SIZE - 1][x][3] = 255;

				data[x][DEFAULT_SIZE - 1][0] =
					data[x][DEFAULT_SIZE - 1][1] =
					data[x][DEFAULT_SIZE - 1][2] =
					data[x][DEFAULT_SIZE - 1][3] = 255;
			}
		}
		else {
			for (y = 0; y < DEFAULT_SIZE; y++) {
				for (x = 0; x < DEFAULT_SIZE; x++) {
					data[y][x][0] = 0;
					data[y][x][1] = 0;
					data[y][x][2] = 0;
					data[y][x][3] = 0;
				}
			}
		}

		image->GenerateImage(
			(byte *)data,
			DEFAULT_SIZE, DEFAULT_SIZE,
			TEXTURE_FILTER_DEFAULT, TEXTURE_REPEAT_REPEAT, TD_DEFAULT);
	}

	/*
	================
	BitsForFormat
	================
	*/
	int BitsForFormat(TextureFormat format) {
		switch (format) {
		case TEXTURE_FORMAT_NONE:		return 0;
		case TEXTURE_FORMAT_RGBA8:		return 32;
		case TEXTURE_FORMAT_XRGB8:		return 32;
		case TEXTURE_FORMAT_RGB565:	return 16;
		case TEXTURE_FORMAT_L8A8:		return 16;
		case TEXTURE_FORMAT_ALPHA:		return 8;
		case TEXTURE_FORMAT_LUM8:		return 8;
		case TEXTURE_FORMAT_INT8:		return 8;
		case TEXTURE_FORMAT_DXT1:		return 4;
		case TEXTURE_FORMAT_DXT5:		return 8;
		case TEXTURE_FORMAT_DEPTH:		return 32;
		case TEXTURE_FORMAT_X16:		return 16;
		case TEXTURE_FORMAT_Y16_X16:	return 32;
		default:
			assert(0);
			return 0;
		}
	}

	/*
	====================
	VK_GetFormatFromTextureFormat
	====================
	*/
	VkFormat VK_GetFormatFromTextureFormat(const TextureFormat format) {
		switch (format) {
		case TEXTURE_FORMAT_RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
		case TEXTURE_FORMAT_XRGB8: return VK_FORMAT_R8G8B8_UNORM;
		case TEXTURE_FORMAT_ALPHA: return VK_FORMAT_R8_UNORM;
		case TEXTURE_FORMAT_L8A8: return VK_FORMAT_R8G8_UNORM;
		case TEXTURE_FORMAT_LUM8: return VK_FORMAT_R8_UNORM;
		case TEXTURE_FORMAT_INT8: return VK_FORMAT_R8_UNORM;
		case TEXTURE_FORMAT_DXT1: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
		case TEXTURE_FORMAT_DXT5: return VK_FORMAT_BC3_UNORM_BLOCK;
		case TEXTURE_FORMAT_DEPTH: return m_device_context.depth_format;
		case TEXTURE_FORMAT_X16: return VK_FORMAT_R16_UNORM;
		case TEXTURE_FORMAT_Y16_X16: return VK_FORMAT_R16G16_UNORM;
		case TEXTURE_FORMAT_RGB565: return VK_FORMAT_R5G6B5_UNORM_PACK16;
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}



	void R_LoadImage(const std::string name, std::vector<byte>& pic, int * width, int * height, int64 * timestamp, bool makePowerOf2)
	{
		std::string cname;
		if (pic.data()) {
			pic.clear();
		}
		if (timestamp) {
			*timestamp = FILE_NOT_FOUND_TIMESTAMP;
		}
		if (width) {
			*width = 0;
		}
		if (height) {
			*height = 0;
		}

		if (name.length() > 4)  //is it have at leat extension size + one  like a.jpg
		{
			//FileUtility::LoadTextureDataFromFile(name.c_str(), 0, pic, width, height, 0, 0);
		}
	}

	void R_LoadImageProgram(std::string &name, std::vector<byte> pic, int * width, int * height, int64 * timestamps, TextureUsage * usage)
	{
		parseBuffer[0] = 0;

		if (timestamps) {
			*timestamps = 0;
		}
		R_ParseImageProgram_r(name, pic, width, height, timestamps, usage);


	}

	/*
	====================
	VK_GetComponentMappingFromTextureFormat
	====================
	*/
	VkComponentMapping VK_GetComponentMappingFromTextureFormat(const TextureFormat format, TextureColor color) {
		VkComponentMapping componentMapping = {
			VK_COMPONENT_SWIZZLE_ZERO,
			VK_COMPONENT_SWIZZLE_ZERO,
			VK_COMPONENT_SWIZZLE_ZERO,
			VK_COMPONENT_SWIZZLE_ZERO
		};

		if (color == COLOR_FORMAT_GREEN_ALPHA) {
			componentMapping.r = VK_COMPONENT_SWIZZLE_ONE;
			componentMapping.g = VK_COMPONENT_SWIZZLE_ONE;
			componentMapping.b = VK_COMPONENT_SWIZZLE_ONE;
			componentMapping.a = VK_COMPONENT_SWIZZLE_G;
			return componentMapping;
		}

		switch (format) {
		case TEXTURE_FORMAT_LUM8:
			componentMapping.r = VK_COMPONENT_SWIZZLE_R;
			componentMapping.g = VK_COMPONENT_SWIZZLE_R;
			componentMapping.b = VK_COMPONENT_SWIZZLE_R;
			componentMapping.a = VK_COMPONENT_SWIZZLE_ONE;
			break;
		case TEXTURE_FORMAT_L8A8:
			componentMapping.r = VK_COMPONENT_SWIZZLE_R;
			componentMapping.g = VK_COMPONENT_SWIZZLE_R;
			componentMapping.b = VK_COMPONENT_SWIZZLE_R;
			componentMapping.a = VK_COMPONENT_SWIZZLE_G;
			break;
		case TEXTURE_FORMAT_ALPHA:
			componentMapping.r = VK_COMPONENT_SWIZZLE_ONE;
			componentMapping.g = VK_COMPONENT_SWIZZLE_ONE;
			componentMapping.b = VK_COMPONENT_SWIZZLE_ONE;
			componentMapping.a = VK_COMPONENT_SWIZZLE_R;
			break;
		case TEXTURE_FORMAT_INT8:
			componentMapping.r = VK_COMPONENT_SWIZZLE_R;
			componentMapping.g = VK_COMPONENT_SWIZZLE_R;
			componentMapping.b = VK_COMPONENT_SWIZZLE_R;
			componentMapping.a = VK_COMPONENT_SWIZZLE_R;
			break;
		default:
			componentMapping.r = VK_COMPONENT_SWIZZLE_R;
			componentMapping.g = VK_COMPONENT_SWIZZLE_G;
			componentMapping.b = VK_COMPONENT_SWIZZLE_B;
			componentMapping.a = VK_COMPONENT_SWIZZLE_A;
			break;
		}

		return componentMapping;
	}

	Image::Image(const char * name) : m_image_name(name) {
		m_bIsSwapChainImage = false;
		m_internalFormat = VK_FORMAT_UNDEFINED;
		m_image = VK_NULL_HANDLE;
		m_view = VK_NULL_HANDLE;
		m_layout = VK_IMAGE_LAYOUT_GENERAL;
		m_sampler = VK_NULL_HANDLE;
		m_generatorFunction = NULL;
		m_filter = TEXTURE_FILTER_DEFAULT;
		m_repeat = TEXTURE_REPEAT_REPEAT;
		m_usage = TD_DEFAULT;
		//m_cubeFiles = CF_2D;

		m_referencedOutsideLevelLoad = false;
		m_levelLoadReferenced = false;
		m_sourceFileTime = FILE_NOT_FOUND_TIMESTAMP;
		m_binaryFileTime = FILE_NOT_FOUND_TIMESTAMP;
		m_refCount = 0;

		//this may need better way
		//m_allocation_garbage.resize(NUM_FRAME_DATA);
	   /* m_image_garbage.resize(NUM_FRAME_DATA);
		static std::vector< VkImageView >		m_view_garbage[NUM_FRAME_DATA];
		static std::vector< VkSampler >		m_sampler_garbage[NUM_FRAME_DATA];*/
	}

	Image::~Image() {
		if (!m_bIsSwapChainImage) {
			PurgeImage();
		}
	}

	inline void Image::DerivedOperations()
	{
		if (m_operations.format == TEXTURE_FORMAT_NONE) {
			m_operations.colorFormat = COLOR_FORMAT_DEFAULT;

			switch (m_usage) {
			case TD_COVERAGE:
				m_operations.format = TEXTURE_FORMAT_DXT1;
				m_operations.colorFormat = COLOR_FORMAT_GREEN_ALPHA;
				break;
			case TD_DEPTH:
				m_operations.format = TEXTURE_FORMAT_DEPTH;
				break;
			case TD_DIFFUSE:
				// TD_DIFFUSE gets only set to when its a diffuse texture for an interaction
				m_operations.gammaMips = true;
				m_operations.format = TEXTURE_FORMAT_DXT5;
				m_operations.colorFormat = COLOR_FORMAT_YCOCG_DXT5;
				break;
			case TD_SPECULAR:
				m_operations.gammaMips = true;
				m_operations.format = TEXTURE_FORMAT_DXT1;
				m_operations.colorFormat = COLOR_FORMAT_DEFAULT;
				break;
			case TD_DEFAULT:
				m_operations.gammaMips = true;
				m_operations.format = TEXTURE_FORMAT_DXT5;
				m_operations.colorFormat = COLOR_FORMAT_DEFAULT;
				break;
			case TD_BUMP:
				m_operations.format = TEXTURE_FORMAT_DXT5;
				m_operations.colorFormat = COLOR_FORMAT_NORMAL_DXT5;
				break;
			case TD_FONT:
				m_operations.format = TEXTURE_FORMAT_DXT1;
				m_operations.colorFormat = COLOR_FORMAT_GREEN_ALPHA;
				m_operations.numLevels = 4; // We only support 4 levels because we align to 16 in the exporter
				m_operations.gammaMips = true;
				break;
			case TD_LIGHT:
				m_operations.format = TEXTURE_FORMAT_RGB565;
				m_operations.gammaMips = true;
				break;
			case TD_LOOKUP_TABLE_MONO:
				m_operations.format = TEXTURE_FORMAT_INT8;
				break;
			case TD_LOOKUP_TABLE_ALPHA:
				m_operations.format = TEXTURE_FORMAT_ALPHA;
				break;
			case TD_LOOKUP_TABLE_RGB1:
			case TD_LOOKUP_TABLE_RGBA:
				m_operations.format = TEXTURE_FORMAT_RGBA8;
				break;
			default:
				assert(false);
				m_operations.format = TEXTURE_FORMAT_RGBA8;
			}
		}
	}

	void Image::SetSamplerState(TextureFilter filter, TextureRepeat repeat)
	{
	}

	void Image::UploadScratchImage(const byte * data, int cols, int rows)
	{
	}

	void Image::AllocImage(const ImageOperations & imgOpts, TextureFilter filter, TextureRepeat repeat)
	{
		m_filter = filter;
		m_repeat = repeat;
		m_operations = imgOpts;
		DerivedOperations();
		AllocImage();
	}

	void Image::PurgeImage()
	{
		//it does nor look legal    
		//this push_pack by index does not look correct

		if (m_sampler != VK_NULL_HANDLE) {
			m_sampler_garbage[m_garbage_index].push_back(m_sampler);  //it does nor look legal  
			m_sampler = VK_NULL_HANDLE;
		}

		if (m_image != VK_NULL_HANDLE) {
			m_allocation_garbage[m_garbage_index].push_back(m_allocation);
			m_view_garbage[m_garbage_index].push_back(m_view);
			m_image_garbage[m_garbage_index].push_back(m_image);

#if defined( ID_USE_AMD_ALLOCATOR )
			m_allocation = NULL;
#else
			m_allocation = VulkanAllocation();
#endif

			m_view = VK_NULL_HANDLE;
			m_image = VK_NULL_HANDLE;
		}
	}

	bool Image::IsLoaded() const {
		return m_image != VK_NULL_HANDLE; // TODO_VK maybe do something better than this.
	}

	void Image::GenerateImage(const byte * pic, int width, int height, TextureFilter filter, TextureRepeat repeat, TextureUsage usage)
	{
		PurgeImage();

		m_filter = filter;
		m_repeat = repeat;
		m_usage = usage;
		m_cubeFiles = CUBE_FILE_2D;

		m_operations.textureType = TEXTURE_TYPE_2D;
		m_operations.width = width;
		m_operations.height = height;
		m_operations.numLevels = 1;
		DerivedOperations();

		BinaryImage im(GetName());
		im.Load2DFromMemory(
			width, height, pic,
			m_operations.numLevels,
			m_operations.format,
			m_operations.colorFormat,
			m_operations.gammaMips);

		AllocImage();

		for (int i = 0; i < im.NumImages(); ++i) {
			const bimageImage_t & img = im.GetImageHeader(i);
			const byte * data = im.GetImageData(i);
			SubImageUpload(img.level, 0, 0, img.destZ, img.width, img.height, data);
		}
	}

	void Image::GenerateCubeImage(const byte * pic[6], int size, TextureFilter filter, TextureUsage usage)
	{
	}

	void Image::ActuallyLoadImage(bool fromBackEnd)
	{
		// this is the ONLY place m_generatorFunction will ever be called
		if (m_generatorFunction) {
			m_generatorFunction(this);
			return;
		}

		//com_productionMode.GetInteger() 
		if (true != 0) {  //if speacial use in this case nono
			m_sourceFileTime = FILE_NOT_FOUND_TIMESTAMP;
			if (m_cubeFiles != CUBE_FILE_2D) {
				m_operations.textureType = TEXTURE_TYPE_CUBIC;
				m_repeat = TEXTURE_REPEAT_CLAMP;
			}
		}
		else
		{
			if (m_cubeFiles != CUBE_FILE_2D) {
				m_operations.textureType = TEXTURE_TYPE_CUBIC;
				m_repeat = TEXTURE_REPEAT_CLAMP;
				//R_LoadCubeImages(GetName(), m_cubeFiles, NULL, NULL, &m_sourceFileTime);
			}
			else {
				m_operations.textureType = TEXTURE_TYPE_2D;
				//(GetName(), NULL, NULL, NULL, &m_sourceFileTime, &m_usage);

				//TDO load jpg or tga file at this piont
			}

		}

		DerivedOperations();

		std::string temp{ GetName() };

		std::string generatedName = "BlackAndWhite";    //temp file name while finding how to generrate names
		GetGeneratedName(generatedName, m_usage, m_cubeFiles);

		BinaryImage im(generatedName);

		//TDO 
		m_binaryFileTime = im.LoadFromGeneratedFile(m_sourceFileTime);
	}

	void Image::ActuallyLoadImage()
	{
		//TDO working progress  --  change to as needed
		m_cubeFiles = CUBE_FILE_2D;
		m_repeat = TEXTURE_REPEAT_CLAMP;
		m_operations.textureType = TEXTURE_TYPE_2D;
		DerivedOperations();

		int width , height; //, image_data_size;
		std::vector<unsigned char> image_data;
		Utility::LoadTextureDataFromFile(m_image_name.c_str(), Utility::IMAGE_RGB_ALPHA, image_data, &width, &height);

		GenerateImage((byte*)image_data.data(), width, height, m_filter, m_repeat, m_usage);


	}

	void Image::GetGeneratedName(std::string & _name, const TextureUsage & _usage, const CubeFiles & _cube)
	{
		std::string extention = ".tga";   //name should ba;a;a

		std::string usage("01");
		std::string cube("02");


		_name += usage + cube + extention;


	}

	void Image::CreateFromSwapImage(VkImage image, VkImageView imageView, VkFormat format, const VkExtent2D & extent) {
		m_image = image;
		m_view = imageView;
		m_internalFormat = format;
		m_operations.textureType = TEXTURE_TYPE_2D;
		m_operations.format = TEXTURE_FORMAT_RGBA8;
		m_operations.numLevels = 1;
		m_operations.width = extent.width;
		m_operations.height = extent.height;
		m_bIsSwapChainImage = true;

		// TODO_VK may need to setup more state here.
	}

	void Image::SubImageUpload(int mipLevel, int x, int y, int z, int width, int height, const void * pic, int pixelPitch)
	{
		assert(x >= 0 && y >= 0 && mipLevel >= 0 && width >= 0 && height >= 0 && mipLevel < m_operations.numLevels);

		if (IsCompressed()) {
			width = (width + 3) & ~3;
			height = (height + 3) & ~3;
		}

		int size = width * height * BitsForFormat(m_operations.format) / 8;

		VkBuffer buffer;
		VkCommandBuffer commandBuffer;
		int offset = 0;
		byte * data = staging_buffer.Stage(size, 16, commandBuffer, buffer, offset);
		if (m_operations.format == TEXTURE_FORMAT_RGB565) {
			byte * imgData = (byte *)pic;
			for (int i = 0; i < size; i += 2) {
				data[i] = imgData[i + 1];
				data[i + 1] = imgData[i];
			}
		}
		else {
			memcpy(data, pic, size);
		}

		VkBufferImageCopy imgCopy = {};
		imgCopy.bufferOffset = offset;
		imgCopy.bufferRowLength = pixelPitch;
		imgCopy.bufferImageHeight = height;
		imgCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imgCopy.imageSubresource.layerCount = 1;
		imgCopy.imageSubresource.mipLevel = mipLevel;
		imgCopy.imageSubresource.baseArrayLayer = z;
		imgCopy.imageOffset.x = x;
		imgCopy.imageOffset.y = y;
		imgCopy.imageOffset.z = 0;
		imgCopy.imageExtent.width = width;
		imgCopy.imageExtent.height = height;
		imgCopy.imageExtent.depth = 1;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = m_operations.numLevels;
		barrier.subresourceRange.baseArrayLayer = z;
		barrier.subresourceRange.layerCount = 1;

		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgCopy);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


	}
	void Image::CreateSampler() {
		VkSamplerCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.maxAnisotropy = 1.0f;
		createInfo.anisotropyEnable = VK_FALSE;
		createInfo.compareEnable = (m_operations.format == TEXTURE_FORMAT_DEPTH);
		createInfo.compareOp = (m_operations.format == TEXTURE_FORMAT_DEPTH) ? VK_COMPARE_OP_LESS_OR_EQUAL : VK_COMPARE_OP_NEVER;

		switch (m_filter) {
		case TEXTURE_FILTER_DEFAULT:
		case TEXTURE_FILTER_LINEAR:
			createInfo.minFilter = VK_FILTER_LINEAR;
			createInfo.magFilter = VK_FILTER_LINEAR;
			createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case TEXTURE_FILTER_NEAREST:
			createInfo.minFilter = VK_FILTER_NEAREST;
			createInfo.magFilter = VK_FILTER_NEAREST;
			createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		default:
			ERROR("Image::CreateSampler: unrecognized texture filter %d", m_filter);
		}

		switch (m_repeat) {
		case TEXTURE_REPEAT_REPEAT:
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		case TEXTURE_REPEAT_CLAMP:
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case TEXTURE_REPEAT_CLAMP_TO_ZERO_ALPHA:
			createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;
		case TEXTURE_REPEAT_CLAMP_TO_ZERO:
			createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			break;
		default:
			ERROR("Image::CreateSampler: unrecognized texture repeat mode %d", m_repeat);
		}

		VK_CHECK(vkCreateSampler(m_device_context.device, &createInfo, NULL, &m_sampler));
	}

	void Image::EmptyGarbage() {
		m_garbage_index = (m_garbage_index + 1) % NUM_FRAME_DATA;

#if defined( ID_USE_AMD_ALLOCATOR )
		idList< VmaAllocation > & allocationsToFree = m_allocationGarbage[m_garbageIndex];
#else
		static std::vector< VulkanAllocation > & allocationsToFree = m_allocation_garbage[m_garbage_index];
#endif
		static std::vector< VkImage > & imagesToFree = m_image_garbage[m_garbage_index];
		static std::vector< VkImageView > & viewsToFree = m_view_garbage[m_garbage_index];
		static std::vector< VkSampler > & samplersToFree = m_sampler_garbage[m_garbage_index];

#if defined( ID_USE_AMD_ALLOCATOR )
		const int numAllocations = allocationsToFree.Num();
		for (int i = 0; i < numAllocations; ++i) {
			vmaDestroyImage(vmaAllocator, imagesToFree[i], allocationsToFree[i]);
		}
#else

		for each(auto allocation in allocationsToFree)
		{
			vulkan_allocator.Free(allocation);
		}

		for each(auto image in imagesToFree)
		{
			vkDestroyImage(m_device_context.device, image, NULL);
		}


#endif


		for each(auto view in viewsToFree)
		{
			vkDestroyImageView(m_device_context.device, view, NULL);
		}

		for each(auto sampler in samplersToFree)
		{
			vkDestroySampler(m_device_context.device, sampler, NULL);

		}



		allocationsToFree.clear();
		imagesToFree.clear();
		viewsToFree.clear();
		samplersToFree.clear();

	}

	void Image::AllocImage() {
		PurgeImage();

		m_internalFormat = VK_GetFormatFromTextureFormat(m_operations.format);

		// Create Sampler
		CreateSampler();

		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
		if (m_operations.format == TEXTURE_FORMAT_DEPTH) {
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else {
			usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		// Create Image
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.flags = (m_operations.textureType == TEXTURE_TYPE_CUBIC) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = m_internalFormat;
		imageCreateInfo.extent.width = m_operations.width;
		imageCreateInfo.extent.height = m_operations.height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = m_operations.numLevels;
		imageCreateInfo.arrayLayers = (m_operations.textureType == TEXTURE_TYPE_CUBIC) ? 6 : 1;
		imageCreateInfo.samples = static_cast<VkSampleCountFlagBits>(m_operations.samples);
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = usageFlags;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

#if defined( ID_USE_AMD_ALLOCATOR )
		VmaMemoryRequirements vmaReq = {};
		vmaReq.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		ID_VK_CHECK(vmaCreateImage(vmaAllocator, &imageCreateInfo, &vmaReq, &m_image, &m_allocation, NULL));
#else
		VK_CHECK(vkCreateImage(m_device_context.device, &imageCreateInfo, NULL, &m_image));

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(m_device_context.device, m_image, &memoryRequirements);

		m_allocation = vulkan_allocator.Allocate(
			memoryRequirements.size,
			memoryRequirements.alignment,
			memoryRequirements.memoryTypeBits,
			VULKAN_MEMORY_USAGE_GPU_ONLY,
			VULKAN_ALLOCATION_TYPE_IMAGE_OPTIMAL);

		VK_CHECK(vkBindImageMemory(m_device_context.device, m_image, m_allocation.deviceMemory, m_allocation.offset));
#endif

		// Create Image View
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = m_image;
		viewCreateInfo.viewType = (m_operations.textureType == TEXTURE_TYPE_CUBIC) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = m_internalFormat;
		viewCreateInfo.components = VK_GetComponentMappingFromTextureFormat(m_operations.format, m_operations.colorFormat);
		viewCreateInfo.subresourceRange.aspectMask = (m_operations.format == TEXTURE_FORMAT_DEPTH) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.levelCount = m_operations.numLevels;
		viewCreateInfo.subresourceRange.layerCount = (m_operations.textureType == TEXTURE_TYPE_CUBIC) ? 6 : 1;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;

		VK_CHECK(vkCreateImageView(m_device_context.device, &viewCreateInfo, NULL, &m_view));
	}

	void ImageManager::Initialize()
	{
		m_images.resize(32);
		//m_imageHash.ResizeIndex(1024);
		CreateIntrinsicImages();
	}

	/*
===============
ImageFromFile

Finds or loads the given image, always returning a valid image pointer.
Loading of the image may be deferred for dynamic loading.
==============
*/

	Image * ImageManager::ImageFromFile(const std::string  name, TextureFilter filter, TextureRepeat repeat, TextureUsage usage, CubeFiles cubeMap)
	{

		std::string _name = name;

		if (!name.c_str() || !name[0] || strcmp(name.c_str(), "default") == 0 || strcmp(name.c_str(), "_default") == 0) {
			//declManager->MediaPrint("DEFAULTED\n");
			return global_images->m_defaultImage;
		}

		

		//
		// create a new image
		//
		Image	* image = AllocImage(name.c_str());
		image->m_cubeFiles = cubeMap;
		image->m_usage = usage;
		image->m_filter = filter;
		image->m_repeat = repeat;

		image->m_levelLoadReferenced = true;


		

		image->ActuallyLoadImage(false);	// load is from front end



		return image;
	}

	Image * ImageManager::ImageFromFile2(const std::string name, TextureFilter filter, TextureRepeat repeat, TextureUsage usage, CubeFiles cubeMap)
	{
		std::string _name = name;

		if (!name.c_str() || !name[0] || strcmp(name.c_str(), "default") == 0 || strcmp(name.c_str(), "_default") == 0) {
			//declManager->MediaPrint("DEFAULTED\n");
			return global_images->m_defaultImage;
		}

		Image	* image = AllocImage(name.c_str());
		image->m_cubeFiles = cubeMap;
		image->m_usage = usage;
		image->m_filter = filter;
		image->m_repeat = repeat;

		image->m_levelLoadReferenced = true;

		image->ActuallyLoadImage();	// load is from front end



		return image;

	}

	Image * ImageManager::ScratchImage(const char * name, const ImageOperations & opts)
	{
		if (!name || !name[0]) {
			ERROR("idImageManager::ScratchImage");
		}

		Image * image = GetImage(name);
		if (image == NULL) {
			image = AllocImage(name);
		}
		else {
			image->PurgeImage();
		}

		image->m_operations = opts;
		image->AllocImage();
		image->m_referencedOutsideLevelLoad = true;

		return image;
	}

	Image * ImageManager::ImageFromFunction(const char * name, void(*generatorFunction)(Image *image))
	{
		
		Image	* image = AllocImage(name);

		image->m_generatorFunction = generatorFunction;

		// check for precompressed, load is from the front end
		image->m_referencedOutsideLevelLoad = true;
		image->ActuallyLoadImage(false);

		return image;
	}



	void ImageManager::CreateIntrinsicImages()
	{
		m_defaultImage = ImageFromFunction("_default", DefaultImage);
		m_scratchImage = ImageFromFunction("_scratch", R_RGBA8Image);
	}

	Image * ImageManager::AllocImage(const char * name)
	{
		if (strlen(name) >= MAX_IMAGE_NAME) {
			ERROR("idImageManager::AllocImage: \"%s\" is too long\n", name);
		}



		//int hash = m_imageHash.FileNameHash(name);

		Image * image = new (TAG_IMAGE) Image(name);
		m_images.push_back(image);

		//m_imageHash.Add(hash, m_images.size());

		return image;
	}

	Image * ImageManager::GetImage(const char * _name) const
	{
		if (!_name || !_name[0] || strcmp(_name, "default") == 0 || strcmp(_name, "_default") == 0) {
			//declManager->MediaPrint("DEFAULTED\n");
			return global_images->m_scratchImage;
		}

		return nullptr;
	}

	void BinaryImage::Load2DFromMemory(int width, int height, const byte * pic_const, int numLevels, TextureFormat & textureFormat, TextureColor & colorFormat, bool gammaMips)
	{
		//TDO this is place the discover what image is to 3D hardware and create one bytes pass to this function
		//Gooo=d luck

		fileData.textureType = TEXTURE_TYPE_2D;
		fileData.format = textureFormat;
		fileData.colorFormat = colorFormat;
		fileData.width = width;
		fileData.height = height;
		fileData.numLevels = numLevels;


		//TDO if data came from file it should come here to create and image that cam be pass
		//3d device either compress or raw 

		byte * pic = (byte *)Mem_Alloc(width * height * 4, TAG_TEMP);
		memcpy(pic, pic_const, width * height * 4);

		int	scaledWidth = width;
		int scaledHeight = height;
		images.resize(numLevels);
		for (int level = 0; level < images.size(); level++) {
			BinaryImageData &img = images[level];


			// Images that are going to be DXT compressed and aren't multiples of 4 need to be 
			// padded out before compressing.
			byte * dxtPic = pic;
			int	dxtWidth = 0;
			int	dxtHeight = 0;


			img.level = level;
			img.destZ = 0;
			img.width = scaledWidth;
			img.height = scaledHeight;

			//==========================   if does not need scale DXT

			dxtWidth = scaledWidth;
			dxtHeight = scaledHeight;

			//=======================

			if (textureFormat == TEXTURE_FORMAT_DXT5) {
				idDxtEncoder dxt;
				img.Alloc(dxtWidth * dxtHeight);

				if (colorFormat == COLOR_FORMAT_NORMAL_DXT5) {
					if (false /*image_highQualityCompression.GetBool()*/) {
						dxt.CompressNormalMapDXT5HQ(dxtPic, img.data, dxtWidth, dxtHeight);
					}
					else {
						dxt.CompressNormalMapDXT5Fast(dxtPic, img.data, dxtWidth, dxtHeight);
					}
				}
				else {
					fileData.colorFormat = colorFormat = COLOR_FORMAT_DEFAULT;
					if (false /*image_highQualityCompression.GetBool()*/) {
						dxt.CompressImageDXT5HQ(dxtPic, img.data, dxtWidth, dxtHeight);
					}
					else {
						dxt.CompressImageDXT5Fast(dxtPic, img.data, dxtWidth, dxtHeight);
					}
				}


			}
			else {
				fileData.format = textureFormat = TEXTURE_FORMAT_RGBA8;
				img.Alloc(scaledWidth * scaledHeight * 4);
				for (int i = 0; i < img.dataSize; i++) {
					img.data[i] = pic[i];
				}
			}

			//I do not know what these lines are doing
			// if we had to pad to quads, free the padded version
			if (pic != dxtPic) {
				Mem_Free(dxtPic);
				dxtPic = NULL;
			}


			//// downsample for the next level
			//byte * shrunk = NULL;
			//if (gammaMips) {
			//	shrunk = R_MipMapWithGamma(pic, scaledWidth, scaledHeight);
			//}
			//else {
			//	shrunk = R_MipMap(pic, scaledWidth, scaledHeight);
			//}
			//Mem_Free(pic);
			//pic = shrunk;

			//scaledWidth = Max(1, scaledWidth >> 1);
			//scaledHeight = Max(1, scaledHeight >> 1);


		}

		Mem_Free(pic);

	}

	void BinaryImage::LoadCubeFromMemory(int width, const byte * pics[6], int numLevels, TextureFormat & textureFormat, bool gammaMips)
	{
	}

	int64 BinaryImage::LoadFromGeneratedFile(int64 sourceFileTime)
	{
		std::vector<unsigned char> binary_file_content;
		std::string binaryFileName;
		MakeGeneratedFileName(binaryFileName);
		//std::ifstream bFile = file_system->OpenFile(binaryFileName);
		if (!file_system->OpenFile(binaryFileName, binary_file_content)) {
			return FILE_NOT_FOUND_TIMESTAMP;
		}
		if (LoadFromGeneratedFile(binary_file_content, sourceFileTime)) {
			return 0;
		}
		return FILE_NOT_FOUND_TIMESTAMP;
	}

	int64 BinaryImage::WriteGeneratedFile(int64 sourceFileTime)
	{
		return int64();
	}

	void BinaryImage::GetGeneratedFileName(std::string & gfn, const char * imageName)
	{
	}

	void BinaryImage::MakeGeneratedFileName(std::string & gfn)
	{
	}

	bool BinaryImage::LoadFromGeneratedFile(std::vector<unsigned char> content, int64 sourceFileTime)
	{

		if (content.size() <= 0) {
			return false;
		}
		//TDO translate the content to texture file where params go the file data
		fileData.sourceFileTime = sourceFileTime;
		return true;
	}











}