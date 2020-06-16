#pragma once

#include "VulkanRenderProgram.h"
#include "..\Math\DrawVertex.h"
#include "..\VulkanFoundation.h"

#include "..\Graphics.h"


#include "GLState.h"

namespace Graphics
{
	VulkanRenderProgramManager render_program_manager;


	// For GLSL we need to have the names for the renderparms so we can look up 
	// their run time indices within the renderprograms
	const char * GLSLParmNames[RENDERPARM_TOTAL] = {
		"rpVertexColorModulate",
		"rpVertexColorAdd",
		"rpColor",
		"rpAlphaTest",

		"rpMVPmatrixX",
		"rpMVPmatrixY",
		"rpMVPmatrixZ",
		"rpMVPmatrixW",

		"rpTextureMatrixS",
		"rpTextureMatrixT",

		"rpTexGen0S",
		"rpTexGen0T",
		"rpTexGen0Q",
		"rpTexGen0Enabled",


		"rpUser0",
		"rpUser1",
		"rpUser2",
		"rpUser3",
		"rpUser4",
		"rpUser5",
		"rpUser6",
		"rpUser7"
	};

	

	struct vertexLayout_t {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector< VkVertexInputBindingDescription > bindingDesc;
		std::vector< VkVertexInputAttributeDescription > attributeDesc;
	};

	static vertexLayout_t vertexLayouts[NUM_VERTEX_LAYOUTS];

	static shader defaultShader;
	static UniformBuffer emptyUBO;

	static const char * renderProgBindingStrings[BINDING_TYPE_MAX] = {
		"ubo",
		"sampler"
	};


	static VkStencilOpState GetStencilOpState(uint64 stencilBits) {
		VkStencilOpState state = {};

		switch (stencilBits & GLS_STENCIL_OP_FAIL_BITS) {
		case GLS_STENCIL_OP_FAIL_KEEP:		state.failOp = VK_STENCIL_OP_KEEP; break;
		case GLS_STENCIL_OP_FAIL_ZERO:		state.failOp = VK_STENCIL_OP_ZERO; break;
		case GLS_STENCIL_OP_FAIL_REPLACE:	state.failOp = VK_STENCIL_OP_REPLACE; break;
		case GLS_STENCIL_OP_FAIL_INCR:		state.failOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; break;
		case GLS_STENCIL_OP_FAIL_DECR:		state.failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP; break;
		case GLS_STENCIL_OP_FAIL_INVERT:	state.failOp = VK_STENCIL_OP_INVERT; break;
		case GLS_STENCIL_OP_FAIL_INCR_WRAP: state.failOp = VK_STENCIL_OP_INCREMENT_AND_WRAP; break;
		case GLS_STENCIL_OP_FAIL_DECR_WRAP: state.failOp = VK_STENCIL_OP_DECREMENT_AND_WRAP; break;
		}
		switch (stencilBits & GLS_STENCIL_OP_ZFAIL_BITS) {
		case GLS_STENCIL_OP_ZFAIL_KEEP:		state.depthFailOp = VK_STENCIL_OP_KEEP; break;
		case GLS_STENCIL_OP_ZFAIL_ZERO:		state.depthFailOp = VK_STENCIL_OP_ZERO; break;
		case GLS_STENCIL_OP_ZFAIL_REPLACE:	state.depthFailOp = VK_STENCIL_OP_REPLACE; break;
		case GLS_STENCIL_OP_ZFAIL_INCR:		state.depthFailOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; break;
		case GLS_STENCIL_OP_ZFAIL_DECR:		state.depthFailOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP; break;
		case GLS_STENCIL_OP_ZFAIL_INVERT:	state.depthFailOp = VK_STENCIL_OP_INVERT; break;
		case GLS_STENCIL_OP_ZFAIL_INCR_WRAP:state.depthFailOp = VK_STENCIL_OP_INCREMENT_AND_WRAP; break;
		case GLS_STENCIL_OP_ZFAIL_DECR_WRAP:state.depthFailOp = VK_STENCIL_OP_DECREMENT_AND_WRAP; break;
		}
		switch (stencilBits & GLS_STENCIL_OP_PASS_BITS) {
		case GLS_STENCIL_OP_PASS_KEEP:		state.passOp = VK_STENCIL_OP_KEEP; break;
		case GLS_STENCIL_OP_PASS_ZERO:		state.passOp = VK_STENCIL_OP_ZERO; break;
		case GLS_STENCIL_OP_PASS_REPLACE:	state.passOp = VK_STENCIL_OP_REPLACE; break;
		case GLS_STENCIL_OP_PASS_INCR:		state.passOp = VK_STENCIL_OP_INCREMENT_AND_CLAMP; break;
		case GLS_STENCIL_OP_PASS_DECR:		state.passOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP; break;
		case GLS_STENCIL_OP_PASS_INVERT:	state.passOp = VK_STENCIL_OP_INVERT; break;
		case GLS_STENCIL_OP_PASS_INCR_WRAP:	state.passOp = VK_STENCIL_OP_INCREMENT_AND_WRAP; break;
		case GLS_STENCIL_OP_PASS_DECR_WRAP:	state.passOp = VK_STENCIL_OP_DECREMENT_AND_WRAP; break;
		}

		return state;
	}

	/*
	========================
	CreateGraphicsPipeline
	========================
	*/
	static VkPipeline CreateGraphicsPipeline(
		VertexLayoutType vertexLayoutType,
		VkShaderModule vertexShader,
		VkShaderModule fragmentShader,
		VkPipelineLayout pipelineLayout,
		uint64 stateBits) {

		// Pipeline
		vertexLayout_t  vertexLayout = vertexLayouts[vertexLayoutType];

		// Vertex Input
		VkPipelineVertexInputStateCreateInfo vertexInputState = vertexLayout.inputState;
		vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexLayout.bindingDesc.size();
		vertexInputState.pVertexBindingDescriptions = vertexLayout.bindingDesc.data();
		vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexLayout.attributeDesc.size();
		vertexInputState.pVertexAttributeDescriptions = vertexLayout.attributeDesc.data();

		// Input Assembly
		VkPipelineInputAssemblyStateCreateInfo assemblyInputState = {};
		assemblyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyInputState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Rasterization
		VkPipelineRasterizationStateCreateInfo rasterizationState = {};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = (stateBits & GLS_POLYGON_OFFSET) != 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.frontFace = (stateBits & GLS_CLOCKWISE) ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.lineWidth = 1.0f;
		rasterizationState.polygonMode = (stateBits & GLS_POLYMODE_LINE) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;

		switch (stateBits & GLS_CULL_BITS) {
		case GLS_CULL_TWOSIDED:
			rasterizationState.cullMode = VK_CULL_MODE_NONE;
			break;
		case GLS_CULL_BACKSIDED:
			if (stateBits & GLS_MIRROR_VIEW) {
				rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
			}
			else {
				rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
			}
			break;
		case GLS_CULL_FRONTSIDED:
		default:
			if (stateBits & GLS_MIRROR_VIEW) {
				rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
			}
			else {
				rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
			}
			break;
		}

		// Color Blend Attachment
		VkPipelineColorBlendAttachmentState attachmentState = {};
		{
			VkBlendFactor srcFactor = VK_BLEND_FACTOR_ONE;
			switch (stateBits & GLS_SRCBLEND_BITS) {
			case GLS_SRCBLEND_ZERO:					srcFactor = VK_BLEND_FACTOR_ZERO; break;
			case GLS_SRCBLEND_ONE:					srcFactor = VK_BLEND_FACTOR_ONE; break;
			case GLS_SRCBLEND_DST_COLOR:			srcFactor = VK_BLEND_FACTOR_DST_COLOR; break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:	srcFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR; break;
			case GLS_SRCBLEND_SRC_ALPHA:			srcFactor = VK_BLEND_FACTOR_SRC_ALPHA; break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:	srcFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; break;
			case GLS_SRCBLEND_DST_ALPHA:			srcFactor = VK_BLEND_FACTOR_DST_ALPHA; break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:	srcFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA; break;
			}

			VkBlendFactor dstFactor = VK_BLEND_FACTOR_ZERO;
			switch (stateBits & GLS_DSTBLEND_BITS) {
			case GLS_DSTBLEND_ZERO:					dstFactor = VK_BLEND_FACTOR_ZERO; break;
			case GLS_DSTBLEND_ONE:					dstFactor = VK_BLEND_FACTOR_ONE; break;
			case GLS_DSTBLEND_SRC_COLOR:			dstFactor = VK_BLEND_FACTOR_SRC_COLOR; break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:	dstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR; break;
			case GLS_DSTBLEND_SRC_ALPHA:			dstFactor = VK_BLEND_FACTOR_SRC_ALPHA; break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:	dstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; break;
			case GLS_DSTBLEND_DST_ALPHA:			dstFactor = VK_BLEND_FACTOR_DST_ALPHA; break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:	dstFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA; break;
			}

			VkBlendOp blendOp = VK_BLEND_OP_ADD;
			switch (stateBits & GLS_BLENDOP_BITS) {
			case GLS_BLENDOP_MIN: blendOp = VK_BLEND_OP_MIN; break;
			case GLS_BLENDOP_MAX: blendOp = VK_BLEND_OP_MAX; break;
			case GLS_BLENDOP_ADD: blendOp = VK_BLEND_OP_ADD; break;
			case GLS_BLENDOP_SUB: blendOp = VK_BLEND_OP_SUBTRACT; break;
			}

			attachmentState.blendEnable = (srcFactor != VK_BLEND_FACTOR_ONE || dstFactor != VK_BLEND_FACTOR_ZERO);
			attachmentState.colorBlendOp = blendOp;
			attachmentState.srcColorBlendFactor = srcFactor;
			attachmentState.dstColorBlendFactor = dstFactor;
			attachmentState.alphaBlendOp = blendOp;
			attachmentState.srcAlphaBlendFactor = srcFactor;
			attachmentState.dstAlphaBlendFactor = dstFactor;

			// Color Mask
			attachmentState.colorWriteMask = 0;
			attachmentState.colorWriteMask |= (stateBits & GLS_REDMASK) ? 0 : VK_COLOR_COMPONENT_R_BIT;
			attachmentState.colorWriteMask |= (stateBits & GLS_GREENMASK) ? 0 : VK_COLOR_COMPONENT_G_BIT;
			attachmentState.colorWriteMask |= (stateBits & GLS_BLUEMASK) ? 0 : VK_COLOR_COMPONENT_B_BIT;
			attachmentState.colorWriteMask |= (stateBits & GLS_ALPHAMASK) ? 0 : VK_COLOR_COMPONENT_A_BIT;
		}

		// Color Blend
		VkPipelineColorBlendStateCreateInfo colorBlendState = {};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &attachmentState;

		// Depth / Stencil
		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		{
			VkCompareOp depthCompareOp = VK_COMPARE_OP_ALWAYS;
			switch (stateBits & GLS_DEPTHFUNC_BITS) {
			case GLS_DEPTHFUNC_EQUAL:		depthCompareOp = VK_COMPARE_OP_EQUAL; break;
			case GLS_DEPTHFUNC_ALWAYS:		depthCompareOp = VK_COMPARE_OP_ALWAYS; break;
			case GLS_DEPTHFUNC_LESS:		depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; break;
			case GLS_DEPTHFUNC_GREATER:		depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL; break;
			}

			VkCompareOp stencilCompareOp = VK_COMPARE_OP_ALWAYS;
			switch (stateBits & GLS_STENCIL_FUNC_BITS) {
			case GLS_STENCIL_FUNC_NEVER:	stencilCompareOp = VK_COMPARE_OP_NEVER; break;
			case GLS_STENCIL_FUNC_LESS:		stencilCompareOp = VK_COMPARE_OP_LESS; break;
			case GLS_STENCIL_FUNC_EQUAL:	stencilCompareOp = VK_COMPARE_OP_EQUAL; break;
			case GLS_STENCIL_FUNC_LEQUAL:	stencilCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; break;
			case GLS_STENCIL_FUNC_GREATER:	stencilCompareOp = VK_COMPARE_OP_GREATER; break;
			case GLS_STENCIL_FUNC_NOTEQUAL: stencilCompareOp = VK_COMPARE_OP_NOT_EQUAL; break;
			case GLS_STENCIL_FUNC_GEQUAL:	stencilCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL; break;
			case GLS_STENCIL_FUNC_ALWAYS:	stencilCompareOp = VK_COMPARE_OP_ALWAYS; break;
			}

			depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilState.depthTestEnable = VK_TRUE;
			depthStencilState.depthWriteEnable = (stateBits & GLS_DEPTHMASK) == 0;
			depthStencilState.depthCompareOp = depthCompareOp;
			if (m_device_context.gpu.device_features.depthBounds) {
				depthStencilState.depthBoundsTestEnable = (stateBits & GLS_DEPTH_TEST_MASK) != 0;
				depthStencilState.minDepthBounds = 0.0f;
				depthStencilState.maxDepthBounds = 1.0f;
			}
			depthStencilState.stencilTestEnable = (stateBits & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS)) != 0;

			uint32 ref = uint32((stateBits & GLS_STENCIL_FUNC_REF_BITS) >> GLS_STENCIL_FUNC_REF_SHIFT);
			uint32 mask = uint32((stateBits & GLS_STENCIL_FUNC_MASK_BITS) >> GLS_STENCIL_FUNC_MASK_SHIFT);

			if (stateBits & GLS_SEPARATE_STENCIL) {
				depthStencilState.front = GetStencilOpState(stateBits & GLS_STENCIL_FRONT_OPS);
				depthStencilState.front.writeMask = 0xFFFFFFFF;
				depthStencilState.front.compareOp = stencilCompareOp;
				depthStencilState.front.compareMask = mask;
				depthStencilState.front.reference = ref;

				depthStencilState.back = GetStencilOpState((stateBits & GLS_STENCIL_BACK_OPS) >> 12);
				depthStencilState.back.writeMask = 0xFFFFFFFF;
				depthStencilState.back.compareOp = stencilCompareOp;
				depthStencilState.back.compareMask = mask;
				depthStencilState.back.reference = ref;
			}
			else {
				depthStencilState.front = GetStencilOpState(stateBits);
				depthStencilState.front.writeMask = 0xFFFFFFFF;
				depthStencilState.front.compareOp = stencilCompareOp;
				depthStencilState.front.compareMask = mask;
				depthStencilState.front.reference = ref;
				depthStencilState.back = depthStencilState.front;
			}
		}

		// Multisample
		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = m_device_context.sample_count;
		if (m_device_context.super_sampling) {
			multisampleState.sampleShadingEnable = VK_TRUE;
			multisampleState.minSampleShading = 1.0f;
		}

		// Shader Stages
		std::vector< VkPipelineShaderStageCreateInfo > stages;
		VkPipelineShaderStageCreateInfo stage = {};
		stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.pName = "main";

		{
			stage.module = vertexShader;
			stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
			stages.push_back(stage);
		}

		if (fragmentShader != VK_NULL_HANDLE) {
			stage.module = fragmentShader;
			stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			stages.push_back(stage);
		}

		// Dynamic
		std::vector< VkDynamicState > dynamic;
		dynamic.push_back(VK_DYNAMIC_STATE_SCISSOR);
		dynamic.push_back(VK_DYNAMIC_STATE_VIEWPORT);

		if (stateBits & GLS_POLYGON_OFFSET) {
			dynamic.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		}

		if (m_device_context.gpu.device_features.depthBounds && (stateBits & GLS_DEPTH_TEST_MASK)) {
			dynamic.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
		}

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = (uint32_t)dynamic.size();
		dynamicState.pDynamicStates = dynamic.data();

		// Viewport / Scissor
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// Pipeline Create
		VkGraphicsPipelineCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.layout = pipelineLayout;
		createInfo.renderPass = m_device_context.render_pass;
		createInfo.pVertexInputState = &vertexInputState;
		createInfo.pInputAssemblyState = &assemblyInputState;
		createInfo.pRasterizationState = &rasterizationState;
		createInfo.pColorBlendState = &colorBlendState;
		createInfo.pDepthStencilState = &depthStencilState;
		createInfo.pMultisampleState = &multisampleState;
		createInfo.pDynamicState = &dynamicState;
		createInfo.pViewportState = &viewportState;
		createInfo.stageCount = (uint32_t) stages.size();
		createInfo.pStages = stages.data();

		VkPipeline pipeline = VK_NULL_HANDLE;

		VK_CHECK(vkCreateGraphicsPipelines(m_device_context.device, m_device_context.pipeline_cache, 1, &createInfo, NULL, &pipeline));

		return pipeline;
	}





	/*
	=============
	CreateVertexDescriptions
	=============
	*/
	static void CreateVertexDescriptions() {
		VkPipelineVertexInputStateCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.flags = 0;

		VkVertexInputBindingDescription binding = {};
		VkVertexInputAttributeDescription attribute = {};

		{
			vertexLayout_t & layout = vertexLayouts[LAYOUT_DRAW_VERT];
			layout.inputState = createInfo;

			uint32 locationNo = 0;
			uint32 offset = 0;

			binding.stride = sizeof(Math::DrawVertex);
			binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			layout.bindingDesc.push_back(binding);

			// Position
			attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute.location = locationNo++;
			attribute.offset = offset;
			layout.attributeDesc.push_back(attribute);
			offset += sizeof(Math::DrawVertex::xyz);

			// TexCoord
			attribute.format = VK_FORMAT_R16G16_SFLOAT;
			attribute.location = locationNo++;
			attribute.offset = offset;
			layout.attributeDesc.push_back(attribute);
			offset += sizeof(Math::DrawVertex::st);

			// Normal
			attribute.format = VK_FORMAT_R8G8B8A8_UNORM;
			attribute.location = locationNo++;
			attribute.offset = offset;
			layout.attributeDesc.push_back(attribute);
			offset += sizeof(Math::DrawVertex::normal);

			// Tangent
			attribute.format = VK_FORMAT_R8G8B8A8_UNORM;
			attribute.location = locationNo++;
			attribute.offset = offset;
			layout.attributeDesc.push_back(attribute);
			offset += sizeof(Math::DrawVertex::tangent);

			// Color1
			attribute.format = VK_FORMAT_R8G8B8A8_UNORM;
			attribute.location = locationNo++;
			attribute.offset = offset;
			layout.attributeDesc.push_back(attribute);
			offset += sizeof(Math::DrawVertex::color);

			// Color2
			attribute.format = VK_FORMAT_R8G8B8A8_UNORM;
			attribute.location = locationNo++;
			attribute.offset = offset;
			layout.attributeDesc.push_back(attribute);
		}

		//{
		//	vertexLayout_t & layout = vertexLayouts[LAYOUT_DRAW_SHADOW_VERT_SKINNED];
		//	layout.inputState = createInfo;

		//	uint32 locationNo = 0;
		//	uint32 offset = 0;

		//	binding.stride = sizeof(idShadowVertSkinned);
		//	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		//	layout.bindingDesc.Append(binding);

		//	// Position
		//	attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		//	attribute.location = locationNo++;
		//	attribute.offset = offset;
		//	layout.attributeDesc.Append(attribute);
		//	offset += sizeof(idShadowVertSkinned::xyzw);

		//	// Color1
		//	attribute.format = VK_FORMAT_R8G8B8A8_UNORM;
		//	attribute.location = locationNo++;
		//	attribute.offset = offset;
		//	layout.attributeDesc.Append(attribute);
		//	offset += sizeof(idShadowVertSkinned::color);

		//	// Color2
		//	attribute.format = VK_FORMAT_R8G8B8A8_UNORM;
		//	attribute.location = locationNo++;
		//	attribute.offset = offset;
		//	layout.attributeDesc.Append(attribute);
		//}

		{
			//vertexLayout_t & layout = vertexLayouts[LAYOUT_DRAW_SHADOW_VERT];
			//layout.inputState = createInfo;

			//binding.stride = sizeof(idShadowVert);
			//binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			//layout.bindingDesc.Append(binding);

			//// Position
			//attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			//attribute.location = 0;
			//attribute.offset = 0;
			//layout.attributeDesc.Append(attribute);
		}
	}

	/*
	========================
	CreateDescriptorPools
	========================
	*/
	static void CreateDescriptorPools(VkDescriptorPool(&pools)[NUM_FRAME_DATA]) {
		const int numPools = 2;
		VkDescriptorPoolSize poolSizes[numPools];
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = MAX_DESC_UNIFORM_BUFFERS;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = MAX_DESC_IMAGE_SAMPLERS;

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.pNext = NULL;
		poolCreateInfo.maxSets = MAX_DESC_SETS;
		poolCreateInfo.poolSizeCount = numPools;
		poolCreateInfo.pPoolSizes = poolSizes;

		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			VK_CHECK(vkCreateDescriptorPool(m_device_context.device, &poolCreateInfo, NULL, &pools[i]));
		}
	}

	/*
	========================
	GetDescriptorType
	========================
	*/
	static VkDescriptorType GetDescriptorType(rpBinding type) {
		switch (type) {
		case BINDING_TYPE_UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case BINDING_TYPE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		default:
			ERROR("Unknown rpBinding_t %d", static_cast<int>(type));
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	/*
	========================
	CreateDescriptorSetLayout
	========================
	*/
	static void CreateDescriptorSetLayout(
		const shader & vertexShader,
		const shader & fragmentShader,
		VulkanRenderProgram & renderProg) {


		//TDO  evaluate data going in these vectors

		// Descriptor Set Layout
			{
				std::vector< VkDescriptorSetLayoutBinding > layoutBindings;
				VkDescriptorSetLayoutBinding binding = {};
				binding.descriptorCount = 1;

				uint32 bindingId = 0;

				binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				for (int i = 0; i < vertexShader.bindings.size(); ++i) {
					binding.binding = bindingId++;
					binding.descriptorType = GetDescriptorType(vertexShader.bindings[i]);
					renderProg.bindings.push_back(vertexShader.bindings[i]);

					layoutBindings.push_back(binding);
				}

				binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				for (int i = 0; i < fragmentShader.bindings.size(); ++i) {
					binding.binding = bindingId++;
					binding.descriptorType = GetDescriptorType(fragmentShader.bindings[i]);
					renderProg.bindings.push_back(fragmentShader.bindings[i]);

					layoutBindings.push_back(binding);
				}

				VkDescriptorSetLayoutCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				createInfo.bindingCount = (uint32_t)layoutBindings.size();
				createInfo.pBindings = layoutBindings.data();

				vkCreateDescriptorSetLayout(m_device_context.device, &createInfo, NULL, &renderProg.descriptorSetLayout);
			}

			// Pipeline Layout
			{
				VkPipelineLayoutCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				createInfo.setLayoutCount = 1;
				createInfo.pSetLayouts = &renderProg.descriptorSetLayout;

				vkCreatePipelineLayout(m_device_context.device, &createInfo, NULL, &renderProg.pipelineLayout);
			}
	}


	VulkanRenderProgramManager::VulkanRenderProgramManager() : m_current(0),
		m_counter(0),
		m_currentData(0),
		m_currentDescSet(0),
		m_currentParmBufferOffset(0) {

		memset(m_parmBuffers, 0, sizeof(m_parmBuffers));

	}

	void VulkanRenderProgramManager::Initiliaze()
	{
		Utility::Printf("----- Initializing Render Shaders -----\n");

		struct builtinShaders_t {
			int index;
			const char * name;
			rpStage stages;
			VertexLayoutType layout;
		} builtins[1] = {
			/*{ BUILTIN_GUI, "gui", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_COLOR, "color", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_SIMPLESHADE, "simpleshade", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_TEXTURED, "texture", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },*/
			{ BUILTIN_TEXTURE_VERTEXCOLOR, "texture_color", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			/*{ BUILTIN_TEXTURE_VERTEXCOLOR_SKINNED, "texture_color_skinned", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_TEXTURE_TEXGEN_VERTEXCOLOR, "texture_color_texgen", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_INTERACTION, "interaction", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_INTERACTION_SKINNED, "interaction_skinned", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_INTERACTION_AMBIENT, "interactionAmbient", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_INTERACTION_AMBIENT_SKINNED, "interactionAmbient_skinned", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_ENVIRONMENT, "environment", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_ENVIRONMENT_SKINNED, "environment_skinned", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_BUMPY_ENVIRONMENT, "bumpyEnvironment", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_BUMPY_ENVIRONMENT_SKINNED, "bumpyEnvironment_skinned", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },

			{ BUILTIN_DEPTH, "depth", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_DEPTH_SKINNED, "depth_skinned", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_SHADOW, "shadow", SHADER_STAGE_VERTEX, LAYOUT_DRAW_SHADOW_VERT },
			{ BUILTIN_SHADOW_SKINNED, "shadow_skinned", SHADER_STAGE_VERTEX, LAYOUT_DRAW_SHADOW_VERT_SKINNED },
			{ BUILTIN_SHADOW_DEBUG, "shadowDebug", SHADER_STAGE_ALL, LAYOUT_DRAW_SHADOW_VERT },
			{ BUILTIN_SHADOW_DEBUG_SKINNED, "shadowDebug_skinned", SHADER_STAGE_ALL, LAYOUT_DRAW_SHADOW_VERT_SKINNED },

			{ BUILTIN_BLENDLIGHT, "blendlight", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_FOG, "fog", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_FOG_SKINNED, "fog_skinned", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_SKYBOX, "skybox", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_WOBBLESKY, "wobblesky", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_BINK, "bink", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },
			{ BUILTIN_BINK_GUI, "bink_gui", SHADER_STAGE_ALL, LAYOUT_DRAW_VERT },*/
		};
		m_renderProgs.resize(1);    //MAX_BUILTINS

		for (int i = 0; i < 1; i++) {

			size_t vIndex = -1;
			if (builtins[i].stages & SHADER_STAGE_VERTEX) {
				vIndex = FindShader(builtins[i].name, SHADER_STAGE_VERTEX);
			}

			size_t fIndex = -1;
			if (builtins[i].stages & SHADER_STAGE_FRAGMENT) {
				fIndex = FindShader(builtins[i].name, SHADER_STAGE_FRAGMENT);
			}

			VulkanRenderProgram & prog = m_renderProgs[i];
			prog.name = builtins[i].name;
			prog.vertexShaderIndex = vIndex;
			prog.fragmentShaderIndex = fIndex;
			prog.vertexLayoutType = builtins[i].layout;

			CreateDescriptorSetLayout(
				m_shaders[vIndex],
				(fIndex >= 0) ? m_shaders[fIndex] : defaultShader,
				prog);
		}

		//TDO
		m_uniforms.resize(RENDERPARM_TOTAL);
		for (int i = 0; i < m_uniforms.size(); i++)
		{
			m_uniforms[i] = Math::Vector4(0, 0, 0, 0);
		}
		/*PlatZeros(RENDERPARM_TOTAL, m_uniforms)
		
*/
		//TDO enabling some features with render programs loaded like this
		//m_renderProgs[ BUILTIN_TEXTURE_VERTEXCOLOR_SKINNED ].usesJoints = true;

		CreateVertexDescriptions();

		

		// Create Descriptor Pools
		CreateDescriptorPools(m_descriptorPools);

		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			m_parmBuffers[i] = new UniformBuffer();
			m_parmBuffers[i]->AllocBufferObject(NULL, MAX_DESC_SETS * MAX_DESC_SET_UNIFORMS * sizeof(Math::Vector4), BUFFER_USAGE_DYNAMIC);
		}

		// Placeholder: mainly for optionalSkinning
		emptyUBO.AllocBufferObject(NULL, sizeof(Math::Vector4), BUFFER_USAGE_DYNAMIC);
	}

	void VulkanRenderProgramManager::Shutdown()
	{
		// destroy shaders
		for (size_t i = 0; i < m_shaders.size(); ++i) {
			shader & _shader = m_shaders[i];
			vkDestroyShaderModule(m_device_context.device, _shader.module, NULL);
			_shader.module = VK_NULL_HANDLE;
		}

		// destroy pipelines
		for (size_t i = 0; i < m_renderProgs.size(); ++i) {
			VulkanRenderProgram & prog = m_renderProgs[i];

			for (int j = 0; j < prog.pipelines.size(); ++j) {
				vkDestroyPipeline(m_device_context.device, prog.pipelines[j].pipeline, NULL);
			}
			prog.pipelines.clear();

			vkDestroyDescriptorSetLayout(m_device_context.device, prog.descriptorSetLayout, NULL);
			vkDestroyPipelineLayout(m_device_context.device, prog.pipelineLayout, NULL);
		}
		m_renderProgs.clear();


		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			m_parmBuffers[i]->FreeBufferObject();
			delete m_parmBuffers[i];
			m_parmBuffers[i] = NULL;
		}

		emptyUBO.FreeBufferObject();

		for (int i = 0; i < NUM_FRAME_DATA; ++i) {
			//vkFreeDescriptorSets( vkcontext.device, m_descriptorPools[ i ], MAX_DESC_SETS, m_descriptorSets[ i ] );
			vkResetDescriptorPool(m_device_context.device, m_descriptorPools[i], 0);
			vkDestroyDescriptorPool(m_device_context.device, m_descriptorPools[i], NULL);
		}

		memset(m_descriptorSets, 0, sizeof(m_descriptorSets));
		memset(m_descriptorPools, 0, sizeof(m_descriptorPools));

		m_counter = 0;
		m_currentData = 0;
		m_currentDescSet = 0;
	}

	void VulkanRenderProgramManager::StartFrame()
	{
		m_counter++;
		m_currentData = m_counter % NUM_FRAME_DATA;
		m_currentDescSet = 0;
		m_currentParmBufferOffset = 0;

		vkResetDescriptorPool(m_device_context.device, m_descriptorPools[m_currentData], 0);
	}

	const Math::Vector4 & VulkanRenderProgramManager::GetRenderParm(RenderParmaters rp)
	{
		return m_uniforms[rp];
	}

	void VulkanRenderProgramManager::SetRenderParm(RenderParmaters rp, const Math::Vector4 * value)
	{
		

		m_uniforms[rp] = Math::Vector4(value->GetX(), value->GetY(), value->GetZ(), value->GetW());

		/*m_uniforms[rp].SetX( value->GetX());
		m_uniforms[rp].SetY(value->GetY());
		m_uniforms[rp].SetZ(value->GetZ());
		m_uniforms[rp].SetW(value->GetW())*/;


		


	}

	void VulkanRenderProgramManager::SetRenderParms(RenderParmaters rp, const Math::Matrix4 * value)
	{
		/*for (int i = 0; i < num; ++i) {
			SetRenderParm((RenderParmaters)(rp + i), value + (i * 4));
		}*/

		SetRenderParm((RenderParmaters)(rp + 0), &value->GetX());
		SetRenderParm((RenderParmaters)(rp + 1), &value->GetY());
		SetRenderParm((RenderParmaters)(rp + 2), &value->GetZ());
		SetRenderParm((RenderParmaters)(rp + 3), &value->GetW());
	}

	size_t VulkanRenderProgramManager::FindShader(const char * name, rpStage stage)
	{
		std::string shaderName(name);
		//shaderName.StripFileExtension();

		for (int i = 0; i < m_shaders.size(); i++) {
			shader & _shader = m_shaders[i];
			if (_shader.name.compare(shaderName.c_str()) == 0 && _shader.stage == stage) {
				LoadShader(i);
				return i;
			}
		}
		shader _shader;
		_shader.name = shaderName;
		_shader.stage = stage;
		m_shaders.push_back(_shader);

		size_t index =  m_shaders.size()-1;
		LoadShader(index);
		return index;
	}

	void VulkanRenderProgramManager::BindProgram(int index)
	{
		if (m_current == index) {
			return;
		}

		m_current = index;

		Utility::Printf("Binding SPIRV Program %s\n", m_renderProgs[index].name.c_str());
	}

	void VulkanRenderProgramManager::CommitCurrent(uint64 stateBits, VkCommandBuffer commandBuffer)
	{
		VulkanRenderProgram & prog = m_renderProgs[m_current];

		VkPipeline pipeline = prog.GetPipeline(
			stateBits,
			m_shaders[prog.vertexShaderIndex].module,
			prog.fragmentShaderIndex != -1 ? m_shaders[prog.fragmentShaderIndex].module : VK_NULL_HANDLE);

		VkDescriptorSetAllocateInfo setAllocInfo = {};
		setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		setAllocInfo.pNext = NULL;
		setAllocInfo.descriptorPool = m_descriptorPools[m_currentData];
		setAllocInfo.descriptorSetCount = 1;
		setAllocInfo.pSetLayouts = &prog.descriptorSetLayout;

		VK_CHECK(vkAllocateDescriptorSets(m_device_context.device, &setAllocInfo, &m_descriptorSets[m_currentData][m_currentDescSet]));

		VkDescriptorSet descSet = m_descriptorSets[m_currentData][m_currentDescSet];
		m_currentDescSet++;

		int writeIndex = 0;
		int bufferIndex = 0;
		int	imageIndex = 0;
		int bindingIndex = 0;

		VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
		VkDescriptorBufferInfo bufferInfos[MAX_DESC_SET_WRITES];
		VkDescriptorImageInfo imageInfos[MAX_DESC_SET_WRITES];

		int uboIndex = 0;
		UniformBuffer * ubos[3] = { NULL, NULL, NULL };    //this need further evaluation

		UniformBuffer vertParms;
		

	
		if ((prog.vertexShaderIndex >= 0) && (m_shaders[prog.vertexShaderIndex].parmIndices.size() > 0)) {
			AllocParmBlockBuffer(m_shaders[prog.vertexShaderIndex].parmIndices, vertParms);

			ubos[uboIndex++] = &vertParms;
		}

		/*UniformBuffer jointBuffer;
		if (prog.usesJoints && m_device_context.jointCacheHandle > 0) {
			if (!vertexCache.GetJointBuffer(vkcontext.jointCacheHandle, &jointBuffer)) {
				idLib::Error("idRenderProgManager::CommitCurrent: jointBuffer == NULL");
				return;
			}
			assert((jointBuffer.GetOffset() & (vkcontext.gpu.props.limits.minUniformBufferOffsetAlignment - 1)) == 0);

			ubos[uboIndex++] = &jointBuffer;
		}
		else if (prog.optionalSkinning) {
			ubos[uboIndex++] = &emptyUBO;
		}*/


		UniformBuffer fragParms;
		if (prog.fragmentShaderIndex >= 0 && m_shaders[prog.fragmentShaderIndex].parmIndices.size() > 0) {
			AllocParmBlockBuffer(m_shaders[prog.fragmentShaderIndex].parmIndices, fragParms);

			ubos[uboIndex++] = &fragParms;
		}

		for (int i = 0; i < prog.bindings.size(); ++i) {
			rpBinding binding = prog.bindings[i];

			switch (binding) {
			case BINDING_TYPE_UNIFORM_BUFFER: {
				UniformBuffer * ubo = ubos[bufferIndex];

				VkDescriptorBufferInfo & bufferInfo = bufferInfos[bufferIndex++];
				memset(&bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
				bufferInfo.buffer = ubo->GetAPIObject();
				bufferInfo.offset = ubo->GetOffset();
				bufferInfo.range = ubo->GetSize();

				VkWriteDescriptorSet & write = writes[writeIndex++];
				memset(&write, 0, sizeof(VkWriteDescriptorSet));
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = descSet;
				write.dstBinding = bindingIndex++;
				write.descriptorCount = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				write.pBufferInfo = &bufferInfo;

				break;
			}
			case BINDING_TYPE_SAMPLER: {
				Image * image = m_device_context.image_prams[imageIndex];

				VkDescriptorImageInfo & imageInfo = imageInfos[imageIndex++];
				memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
				imageInfo.imageLayout = image->GetLayout();
				imageInfo.imageView = image->GetView();
				imageInfo.sampler = image->GetSampler();

				assert(image->GetView() != VK_NULL_HANDLE);

				VkWriteDescriptorSet & write = writes[writeIndex++];
				memset(&write, 0, sizeof(VkWriteDescriptorSet));
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = descSet;
				write.dstBinding = bindingIndex++;
				write.descriptorCount = 1;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.pImageInfo = &imageInfo;

				break;
			}
			}
		}

		vkUpdateDescriptorSets(m_device_context.device, writeIndex, writes, 0, NULL);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, prog.pipelineLayout, 0, 1, &descSet, 0, NULL);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);



	}

	size_t VulkanRenderProgramManager::FindProgram(const char * name, int vIndex, int fIndex)
	{
		for (int i = 0; i < m_renderProgs.size(); ++i) {
			VulkanRenderProgram & prog = m_renderProgs[i];
			if (prog.vertexShaderIndex == vIndex &&
				prog.fragmentShaderIndex == fIndex) {
				return i;
			}
		}

		VulkanRenderProgram program;
		program.name = name;
		program.vertexShaderIndex = vIndex;
		program.fragmentShaderIndex = fIndex;

		CreateDescriptorSetLayout(m_shaders[vIndex], m_shaders[fIndex], program);

		// HACK: HeatHaze ( optional skinning )
		//{
		//	static const int heatHazeNameNum = 3;
		//	static const char * const heatHazeNames[heatHazeNameNum] = {
		//		"heatHaze",
		//		"heatHazeWithMask",
		//		"heatHazeWithMaskAndVertex"
		//	};
		//	for (int i = 0; i < heatHazeNameNum; ++i) {
		//		// Use the vertex shader name because the renderProg name is more unreliable
		//		if (std::strcmp(m_shaders[vIndex].name.c_str(), heatHazeNames[i]) == 0) {
		//			program.usesJoints = true;
		//			program.optionalSkinning = true;
		//			break;
		//		}
		//	}
		//}
		m_renderProgs.push_back(program);
		size_t index = m_renderProgs.size() - 1;
		return index;
	}

	void VulkanRenderProgramManager::LoadShader(size_t index)
	{
		if (m_shaders[index].module != VK_NULL_HANDLE) {
			return; // Already loaded
		}

		LoadShader(m_shaders[index]);
	}

	void VulkanRenderProgramManager::LoadShader(shader & shader)
	{
		std::string spirvPath;
		std::string layoutPath;

		//"D:\\Development C++\\VULKAN PROJECTS\\VULKAN.Document\\Vulkan.App\\renderprogs"
		spirvPath.append("D:\\Development\\C++\\VULKAN PROJECTS\\VULKAN.Document\\Vulkan.App\\renderprogs\\spirv\\" + shader.name);
		layoutPath.append("D:\\Development\\C++\\VULKAN PROJECTS\\VULKAN.Document\\Vulkan.App\\renderprogs\\vkglsl\\" + shader.name);
		if (shader.stage == SHADER_STAGE_FRAGMENT) {
			spirvPath += ".fspv";
			layoutPath += ".frag.layout";
		}
		else {
			spirvPath += ".vspv";
			layoutPath += ".vert.layout";
		}

		//TDO read data in sperv files only

		std::vector<unsigned char> spirvBuffer;
		
		if (!file_system->OpenFile(spirvPath.c_str(), spirvBuffer))
		{
			ERROR("idRenderProgManager::LoadShader: Unable to load SPIRV shader file %s.", spirvPath.c_str());
		
		}
				

		//void * layoutBuffer = NULL;
		std::vector<std::string> layoutBuffer = file_system->OpenTextFile(layoutPath.c_str());
		if (layoutBuffer.size() <= 0) {
			ERROR("idRenderProgManager::LoadShader: Unable to load layout file %s.", layoutPath.c_str());
		}

		
		std::vector<std::string> parameters = GetParamertsAndBindings(layoutBuffer, "uniforms:");

		for (auto item : parameters)
		{
			int index = -1;
			for (int i = 0; i < RENDERPARM_TOTAL && index == -1; ++i) {
				if (item == GLSLParmNames[i]) {
					index = i;
				}
			}

			if (index == -1) {
				Utility::Printf("Invalid uniform %s", item);
			}


			shader.parmIndices.push_back(static_cast< RenderParmaters >(index));
		}

		std::vector<std::string> bindings = GetParamertsAndBindings(layoutBuffer, "bindings:");


		for (auto item : bindings)
		{
			int index = -1;
			for (int i = 0; i < RENDERPARM_TOTAL && index == -1; ++i) {
				if (item == renderProgBindingStrings[i]) {
					index = i;
				}
			}

			if (index == -1) {
				Utility::Printf("Invalid binding %s", item);
			}


			shader.bindings.push_back(static_cast< rpBinding >(index));
		}


		
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = spirvBuffer.size();
		shaderModuleCreateInfo.pCode = (uint32 *)spirvBuffer.data();

		VK_CHECK(vkCreateShaderModule(m_device_context.device, &shaderModuleCreateInfo, NULL, &shader.module));

		//Mem_Free(layoutBuffer);
		//Mem_Free(spirvBuffer);
	}


	std::vector<std::string> VulkanRenderProgramManager::GetParamertsAndBindings(std::vector<std::string> data, std::string type)
	{
		int start = -1;
		std::string end = "end " + type;
		std::string _type = type;

		std::vector<std::string> result;

		for (auto item : data)
		{
			if (item == _type)
			{
				start = 1;
				_type = "";
			}	
			
			if (item == end)
				start = 0;

			if (start == 1)
			{
				if(item != type)
					result.push_back(item);
			}

		
		}

		return result;
	}

	void VulkanRenderProgramManager::AllocParmBlockBuffer(const std::vector<int>& parmIndices, UniformBuffer & ubo)
	{
		const size_t numParms = parmIndices.size();
		const size_t bytes = ALIGN(numParms * sizeof(Math::Vector4), m_device_context.gpu.device_props.limits.minUniformBufferOffsetAlignment);

		ubo.Reference(*m_parmBuffers[m_currentData], m_currentParmBufferOffset, bytes);

		Math::Vector4 * uniforms = (Math::Vector4 *)ubo.MapBuffer(BUFFER_MAP_WRITE);

		for (int i = 0; i < numParms; ++i) {
			uniforms[i] = render_program_manager.GetRenderParm(static_cast< RenderParmaters >(parmIndices[i]));
		}

		ubo.UnmapBuffer();

		m_currentParmBufferOffset += bytes;
	}

	

	VkPipeline VulkanRenderProgram::GetPipeline(uint64 stateBits, VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		for (int i = 0; i < pipelines.size(); ++i) {
			if (stateBits == pipelines[i].stateBits) {
				return pipelines[i].pipeline;
			}
		}

		VkPipeline pipeline = CreateGraphicsPipeline(vertexLayoutType, vertexShader, fragmentShader, pipelineLayout, stateBits);

		PipelineState pipelineState;
		pipelineState.pipeline = pipeline;
		pipelineState.stateBits = stateBits;
		pipelines.push_back(pipelineState);

		return pipeline;
	}

}
