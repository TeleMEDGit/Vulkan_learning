#pragma once

#include "VulkanAllocateMemory.h"

namespace Graphics
{
	int BitsForFormat(TextureFormat format);

	struct bimageImage_t {
		int		level;
		int		destZ;
		int		width;
		int		height;
		int		dataSize;
		// dataSize bytes follow
	};

	//#pragma pack( push, 1 )
	struct bimageFile_t {
		int64	sourceFileTime;
		int		headerMagic;
		int		textureType;
		int		format;
		int		colorFormat;
		int		width;
		int		height;
		int		numLevels;
		// one or more bimageImage_t structures follow
	};

	class ImageOperations {
	public:
		ImageOperations();

		bool	operator==(const ImageOperations & opts);

		//---------------------------------------------------
		// these determine the physical memory size and layout
		//---------------------------------------------------

		TextureType		textureType;
		TextureFormat		format;
		TextureColor		colorFormat;
		TextureSamples	   samples;
		int					width;
		int					height;			// not needed for cube maps
		int					numLevels;		// if 0, will be 1 for NEAREST / LINEAR filters, otherwise based on size
		bool				gammaMips;		// if true, mips will be generated with gamma correction
		bool				readback;		// 360 specific - cpu reads back from this texture, so allocate with cached memory
	};

	inline bool ImageOperations::operator==(const ImageOperations & opts) {
		return (memcmp(this, &opts, sizeof(*this)) == 0);
	}

	/*
	========================
	idImageOpts::idImageOpts
	========================
	*/
	inline ImageOperations::ImageOperations() {
		format = TEXTURE_FORMAT_NONE;
		colorFormat = COLOR_FORMAT_DEFAULT;
		samples = SAMPLE_1;
		width = 0;
		height = 0;
		numLevels = 0;
		textureType = TEXTURE_TYPE_2D;
		gammaMips = false;
		readback = false;

	};

	class BinaryImage {
	public:
		BinaryImage(const std::string name) : imgName(name) { }
		const char *		GetName() const { return imgName.c_str(); }
		void				SetName(const char *_name) { imgName = _name; }

		void				Load2DFromMemory(int width, int height, const byte * pic_const, int numLevels, TextureFormat & textureFormat, TextureColor & colorFormat, bool gammaMips);
		void				LoadCubeFromMemory(int width, const byte * pics[6], int numLevels, TextureFormat & textureFormat, bool gammaMips);
		int64			LoadFromGeneratedFile(int64 sourceFileTime);
		int64			WriteGeneratedFile(int64 sourceFileTime);

		const bimageFile_t &	GetFileHeader() { return fileData; }

		size_t					NumImages() { return images.size(); }
		const bimageImage_t &	GetImageHeader(int i) const { return images[i]; }
		const byte *			GetImageData(int i) const { return images[i].data; }

		static void  GetGeneratedFileName(std::string & gfn, const char *imageName);


	private:
		std::string				imgName;			// game path, including extension (except for cube maps), may be an image program
		bimageFile_t		fileData;
		class BinaryImageData : public bimageImage_t {
		public:
			byte * data;

			BinaryImageData() : data(NULL) { }
			~BinaryImageData() { Free(); }
			BinaryImageData & operator=(BinaryImageData & other) {
				if (this == &other) {
					return *this;
				}

				Alloc(other.dataSize);
				memcpy(data, other.data, other.dataSize);
				return *this;
			}
			void Free() {
				if (data != NULL) {
					Mem_Free(data);
					data = NULL;
					dataSize = 0;
				}
			}
			void Alloc(int size) {
				Free();
				dataSize = size;
				data = (byte *)Mem_Alloc(size, TAG_CRAP);
			}


		};

		std::vector< BinaryImageData> images;  //

	private:
		void				MakeGeneratedFileName(std::string & gfn);
		bool				LoadFromGeneratedFile(std::vector<unsigned char> content, int64 sourceFileTime);


	};

	class Image
	{
	public:
		Image(const char * name);
		~Image();

#if defined( VULKAN )
		void		CreateFromSwapImage(VkImage image, VkImageView imageView, VkFormat format, const VkExtent2D & extent);
		VkImage		GetImage() const { return m_image; }
		VkImageView	GetView() const { return m_view; }
		VkImageLayout GetLayout() const { return m_layout; }
		VkSampler	GetSampler() const { return m_sampler; }

#endif

		void		AllocImage(const ImageOperations &imgOpts, TextureFilter filter, TextureRepeat repeat);
		void		PurgeImage();

		bool		IsCompressed() const { return (m_operations.format == TEXTURE_FORMAT_DXT1 || m_operations.format == TEXTURE_FORMAT_DXT5); }

		bool		IsLoaded() const;


		const std::string	GetName() const { return m_image_name; }

		// used by callback functions to specify the actual data
		// data goes from the bottom to the top line of the image, as OpenGL expects it
		// These perform an implicit Bind() on the current texture unit
		// FIXME: should we implement cinematics this way, instead of with explicit calls?
		void		GenerateImage(
			const byte * pic,
			int width, int height,
			TextureFilter filter,
			TextureRepeat repeat,
			TextureUsage usage);
		void		GenerateCubeImage(
			const byte *pic[6],
			int size,
			TextureFilter filter,
			TextureUsage usage);
		void(*m_generatorFunction)(Image *image);	// NULL for files

		void		ActuallyLoadImage(bool fromBackEnd);
		void	    ActuallyLoadImage();

		static void	GetGeneratedName(std::string &_name, const TextureUsage &_usage, const CubeFiles &_cube);
	private:

		friend class VulkanFoundation;
		friend class ImageManager;

		// parameters that define this image
		std::string 	m_image_name;				// game path, including extension (except for cube maps), may be an image program
		CubeFiles			m_cubeFiles;			// If this is a cube map, and if so, what kind

		TextureUsage		m_usage;				// Used to determine the type of compression to use
		ImageOperations	    m_operations;					// Parameters that determine the storage method

													// Sampler settings
		TextureFilter		m_filter;
		TextureRepeat		m_repeat;

		bool				m_referencedOutsideLevelLoad;// for determining if it needs to be purged
		bool				m_levelLoadReferenced;	// for determining if it needs to be purged
		SYSTEM_TIME_T			m_sourceFileTime;		// the most recent of all images used in creation, for reloadImages command
		SYSTEM_TIME_T			m_binaryFileTime;		// the time stamp of the binary file

		int					m_refCount;				// overall ref count

		bool				m_bIsSwapChainImage;
		VkFormat			m_internalFormat;
		VkImage				m_image;
		VkImageView			m_view;
		VkImageLayout		m_layout;
		VkSampler			m_sampler;

		inline void		AllocImage();

		void DerivedOperations();

		void		SetSamplerState(TextureFilter filter, TextureRepeat repeat);
		void		UploadScratchImage(const byte * data, int cols, int rows);

		// z is 0 for 2D textures, 0 - 5 for cube maps, and 0 - uploadDepth for 3D textures. Only 
		// one plane at a time of 3D textures can be uploaded. The data is assumed to be correct for 
		// the format, either bytes, halfFloats, floats, or DXT compressed. The data is assumed to 
		// be in OpenGL RGBA format, the consoles may have to reorganize. pixelPitch is only needed 
		// when updating from a source subrect. Width, height, and dest* are always in pixels, so 
		// they must be a multiple of four for dxt data.
		void		SubImageUpload(
			int mipLevel,
			int x, int y, int z,
			int width, int height,
			const void * pic,
			int pixelPitch = 0);

#if defined( VULKAN )
		void		CreateSampler();

		static void EmptyGarbage();
#endif

#if defined( ID_USE_AMD_ALLOCATOR )
		VmaAllocation		m_allocation;
		static std::list< VmaAllocation >		m_allocation_garbage[NUM_FRAME_DATA];
#else
		VulkanAllocation	m_allocation;
		//static std::list< VulkanAllocation > m_allocation_garbage[NUM_FRAME_DATA];
		static std::vector< VulkanAllocation > m_allocation_garbage[NUM_FRAME_DATA];

#endif

		static int							m_garbage_index;
		static std::vector< VkImage >			m_image_garbage[NUM_FRAME_DATA];
		static std::vector< VkImageView >		m_view_garbage[NUM_FRAME_DATA];
		static std::vector< VkSampler >		m_sampler_garbage[NUM_FRAME_DATA];

	};


	class ImageManager {
	public:
		ImageManager() {};
		void Initialize();

		// The callback will be issued immediately, and later if images are reloaded or vid_restart
		// The callback function should call one of the idImage::Generate* functions to fill in the data

		// If the exact combination of parameters has been asked for already, an existing
		// image will be returned, otherwise a new image will be created.
		// Be careful not to use the same image file with different filter / repeat / etc parameters
		// if possible, because it will cause a second copy to be loaded.
		// If the load fails for any reason, the image will be filled in with the default
		// grid pattern.
		// Will automatically execute image programs if needed.
		Image *	ImageFromFile(const std::string name,
			TextureFilter filter, TextureRepeat repeat, TextureUsage usage, CubeFiles cubeMap = CUBE_FILE_2D);
		Image *	ImageFromFile2(const std::string name,
			TextureFilter filter, TextureRepeat repeat, TextureUsage usage, CubeFiles cubeMap = CUBE_FILE_2D);


		// These images are for internal renderer use.  Names should start with "_".
		Image *			ScratchImage(const char * name, const ImageOperations & opts);

		Image *			ImageFromFunction(const char *name, void(*generatorFunction)(Image *image));

		// built-in images
		void				CreateIntrinsicImages();
		Image *			AllocImage(const char *name);
		Image *ImageManager::GetImage(const char *_name) const;


	public:
		Image *			m_defaultImage;
		Image *			m_scratchImage;
		Image *			m_scratch_depth;

		std::vector<Image *> m_images;
		//HashIndex			m_imageHash;

	};

	extern ImageManager	*global_images;

	void LoadImageFromImageFile(Image *image, std::string name);


	void R_LoadImage(const std::string name, std::vector<byte> &pic, int *width, int *height, int64 *timestamp, bool makePowerOf2);

	void R_LoadImageProgram(const std::string *name, std::vector<byte> **pic, int *width, int *height, int64 *timestamps, TextureUsage * usage = NULL);






}
