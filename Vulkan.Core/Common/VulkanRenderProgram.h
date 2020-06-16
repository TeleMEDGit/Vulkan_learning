#pragma once
#include "Common.h"
#include "..\Utility.h"
#include "..\Math\VectorMath.h"
#include "BufferObject.h"


namespace Graphics
{
#define VERTEX_UNIFORM_ARRAY_NAME			"_va_"
#define FRAGMENT_UNIFORM_ARRAY_NAME			"_fa_"

	extern FileSystem* file_system;

	static const int PC_ATTRIB_INDEX_VERTEX = 0;
	static const int PC_ATTRIB_INDEX_NORMAL = 2;
	static const int PC_ATTRIB_INDEX_COLOR = 3;
	static const int PC_ATTRIB_INDEX_COLOR2 = 4;
	static const int PC_ATTRIB_INDEX_ST = 8;
	static const int PC_ATTRIB_INDEX_TANGENT = 9;

	enum VertexMask {
		VERTEX_MASK_XYZ = BIT(PC_ATTRIB_INDEX_VERTEX),
		VERTEX_MASK_ST = BIT(PC_ATTRIB_INDEX_ST),
		VERTEX_MASK_NORMAL = BIT(PC_ATTRIB_INDEX_NORMAL),
		VERTEX_MASK_COLOR = BIT(PC_ATTRIB_INDEX_COLOR),
		VERTEX_MASK_TANGENT = BIT(PC_ATTRIB_INDEX_TANGENT),
		VERTEX_MASK_COLOR2 = BIT(PC_ATTRIB_INDEX_COLOR2),
	};


	enum VertexLayoutType {
		LAYOUT_UNKNOWN = -1,
		LAYOUT_DRAW_VERT,
		LAYOUT_DRAW_SHADOW_VERT,
		LAYOUT_DRAW_SHADOW_VERT_SKINNED,
		NUM_VERTEX_LAYOUTS
	};

	// This enum list corresponds to the global constant register indecies as defined in global.inc for all
	// shaders.  We used a shared pool to keeps things simple.  If something changes here then it also
	// needs to change in global.inc and vice versa
	enum RenderParmaters {
		// For backwards compatibility, do not change the order of the first 17 items
		RENDERPARM_VERTEXCOLOR_MODULATE,
		RENDERPARM_VERTEXCOLOR_ADD,	
		RENDERPARM_COLOR,	
		RENDERPARM_ALPHA_TEST,

		RENDERPARM_MVPMATRIX_X,
		RENDERPARM_MVPMATRIX_Y,
		RENDERPARM_MVPMATRIX_Z,
		RENDERPARM_MVPMATRIX_W,

		RENDERPARM_TEXTUREMATRIX_S,
		RENDERPARM_TEXTUREMATRIX_T,

		RENDERPARM_TEXGEN_0_S,
		RENDERPARM_TEXGEN_0_T,
		RENDERPARM_TEXGEN_0_Q,
		RENDERPARM_TEXGEN_0_ENABLED,
		
		
		RENDERPARM_USER0,
		RENDERPARM_USER1,
		RENDERPARM_USER2,
		RENDERPARM_USER3,
		RENDERPARM_USER4,
		RENDERPARM_USER5,
		RENDERPARM_USER6,
		RENDERPARM_USER7,

		RENDERPARM_TOTAL
	};

	const char * GLSLParmNames[];

	enum rpBuiltIn {
		BUILTIN_GUI,
		BUILTIN_COLOR,
		BUILTIN_SIMPLESHADE,
		BUILTIN_TEXTURED,
		BUILTIN_TEXTURE_VERTEXCOLOR,
		BUILTIN_TEXTURE_VERTEXCOLOR_SKINNED,
		BUILTIN_TEXTURE_TEXGEN_VERTEXCOLOR,
		BUILTIN_INTERACTION,
		BUILTIN_INTERACTION_SKINNED,
		BUILTIN_INTERACTION_AMBIENT,
		BUILTIN_INTERACTION_AMBIENT_SKINNED,
		BUILTIN_ENVIRONMENT,
		BUILTIN_ENVIRONMENT_SKINNED,
		BUILTIN_BUMPY_ENVIRONMENT,
		BUILTIN_BUMPY_ENVIRONMENT_SKINNED,

		BUILTIN_DEPTH,
		BUILTIN_DEPTH_SKINNED,
		BUILTIN_SHADOW,
		BUILTIN_SHADOW_SKINNED,
		BUILTIN_SHADOW_DEBUG,
		BUILTIN_SHADOW_DEBUG_SKINNED,

		BUILTIN_BLENDLIGHT,
		BUILTIN_FOG,
		BUILTIN_FOG_SKINNED,
		BUILTIN_SKYBOX,
		BUILTIN_WOBBLESKY,
		BUILTIN_BINK,
		BUILTIN_BINK_GUI,

		MAX_BUILTINS
	};

	enum rpStage {
		SHADER_STAGE_VERTEX = BIT(0),
		SHADER_STAGE_FRAGMENT = BIT(1),
		SHADER_STAGE_ALL = SHADER_STAGE_VERTEX | SHADER_STAGE_FRAGMENT
	};

	enum rpBinding {
		BINDING_TYPE_UNIFORM_BUFFER,
		BINDING_TYPE_SAMPLER,
		BINDING_TYPE_MAX
	};

	struct shader {
		shader() : module(VK_NULL_HANDLE) {}

		std::string					name;
		rpStage				stage;
		VkShaderModule			module;
		std::vector< rpBinding >	bindings;
		std::vector< int >			parmIndices;
	};


	//class DrawVertex;



	struct VulkanRenderProgram {


		VulkanRenderProgram() :
			usesJoints(false),
			optionalSkinning(false),
			vertexShaderIndex(-1),
			fragmentShaderIndex(-1),
			vertexLayoutType(LAYOUT_DRAW_VERT),
			pipelineLayout(VK_NULL_HANDLE),
			descriptorSetLayout(VK_NULL_HANDLE) {}

		struct PipelineState {
			PipelineState() :
				stateBits(0),
				pipeline(VK_NULL_HANDLE) {
			}

			uint64		stateBits;
			VkPipeline	pipeline;
		};

		VkPipeline GetPipeline(uint64 stateBits, VkShaderModule vertexShader, VkShaderModule fragmentShader);

		std::string						name;
		bool						usesJoints;
		bool						optionalSkinning;
		size_t					    vertexShaderIndex;
		size_t						fragmentShaderIndex;
		VertexLayoutType			vertexLayoutType;
		VkPipelineLayout			pipelineLayout;
		VkDescriptorSetLayout		descriptorSetLayout;
		std::vector< rpBinding>		bindings;
		std::vector<PipelineState >	pipelines;
	

	};

	class VulkanRenderProgramManager {
	public:
		
		VulkanRenderProgramManager();
		void Initiliaze();
		
		void	Shutdown();

		void	StartFrame();

		const Math::Vector4 & GetRenderParm(RenderParmaters rp);
		void	SetRenderParm(RenderParmaters rp, const Math::Vector4 * value);
		void	SetRenderParms(RenderParmaters rp, const Math::Matrix4 * values);

		const VulkanRenderProgram & GetCurrentRenderProg() const { return m_renderProgs[m_current]; }
		size_t		FindShader(const char * name, rpStage stage);
		void	BindProgram(int index);

		void	CommitCurrent(uint64 stateBits, VkCommandBuffer commandBuffer);
		size_t		FindProgram(const char * name, int vIndex, int fIndex);

	private:
		void	LoadShader(size_t index);
		void	LoadShader(shader & shader);

		void	AllocParmBlockBuffer(const std::vector< int > & parmIndices, UniformBuffer & ubo);

	public:
		
		std::vector< VulkanRenderProgram > m_renderProgs;

	private:
		//class friend VulkanFoundation;
		int	m_current;
		std::vector<Math::Vector4> m_uniforms;   //RENDERPARM_TOTAL

		int	m_builtinShaders[MAX_BUILTINS];
		std::vector< shader>	m_shaders;  //enum of tags -> TAG_RENDER

		int					m_counter;
		int					m_currentData;
		int					m_currentDescSet;
		size_t					m_currentParmBufferOffset;
		VkDescriptorPool	m_descriptorPools[NUM_FRAME_DATA];
		VkDescriptorSet		m_descriptorSets[NUM_FRAME_DATA][MAX_DESC_SETS];

		UniformBuffer *	m_parmBuffers[NUM_FRAME_DATA];

		std::vector<std::string> GetParamertsAndBindings(std::vector<std::string> data, std::string type);

	};

	extern VulkanRenderProgramManager render_program_manager;

}