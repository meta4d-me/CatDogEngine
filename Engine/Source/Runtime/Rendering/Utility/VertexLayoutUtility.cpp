#include "Base/NameOf.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Log/Log.h"

#include <cassert>
#include <string>
#include <vector>

namespace
{

constexpr bgfx::Attrib::Enum AllAttribColorTypes[] = {
	bgfx::Attrib::Enum::Color0,
	bgfx::Attrib::Enum::Color1,
	bgfx::Attrib::Enum::Color2,
	bgfx::Attrib::Enum::Color3
};
//constexpr uint32_t MAX_COLOR_COUNT = sizeof(AllAttribColorTypes) / sizeof(bgfx::Attrib::Enum);

constexpr bgfx::Attrib::Enum AllAttribUVTypes[] = {
	bgfx::Attrib::Enum::TexCoord0,
	bgfx::Attrib::Enum::TexCoord1,
	bgfx::Attrib::Enum::TexCoord2,
	bgfx::Attrib::Enum::TexCoord3,
	bgfx::Attrib::Enum::TexCoord4,
	bgfx::Attrib::Enum::TexCoord5,
	bgfx::Attrib::Enum::TexCoord6,
	bgfx::Attrib::Enum::TexCoord7
};

void ConvertVertexLayout(const cd::VertexAttributeLayout& vertexAttributeLayout, bgfx::VertexLayout& outVertexLayout)
{
	bgfx::Attrib::Enum vertexAttribute = bgfx::Attrib::Enum::Count;
	bgfx::AttribType::Enum vertexAttributeValue = bgfx::AttribType::Enum::Count;
	bool normalized = false;

	// Attribute Type
	switch (vertexAttributeLayout.vertexAttributeType)
	{
	case cd::VertexAttributeType::Position:
		vertexAttribute = bgfx::Attrib::Enum::Position;
		break;
	case cd::VertexAttributeType::Normal:
		vertexAttribute = bgfx::Attrib::Enum::Normal;
		normalized = true;
		break;
	case cd::VertexAttributeType::Tangent:
		vertexAttribute = bgfx::Attrib::Enum::Tangent;
		break;
	case cd::VertexAttributeType::Bitangent:
		vertexAttribute = bgfx::Attrib::Enum::Bitangent;
		break;
	case cd::VertexAttributeType::UV:
		vertexAttribute = bgfx::Attrib::Enum::Count;
		for (const bgfx::Attrib::Enum& textCoord : AllAttribUVTypes)
		{
			if (!outVertexLayout.has(textCoord))
			{
				vertexAttribute = textCoord;
				break;
			}
		}
		break;
	case cd::VertexAttributeType::Color:
		vertexAttribute = bgfx::Attrib::Enum::Count;
		for (const bgfx::Attrib::Enum& color : AllAttribColorTypes)
		{
			if (!outVertexLayout.has(color))
			{
				vertexAttribute = color;
				break;
			}
		}
		break;
	case cd::VertexAttributeType::BoneIndex:
		vertexAttribute = bgfx::Attrib::Enum::Indices;
		break;
	case cd::VertexAttributeType::BoneWeight:
		vertexAttribute = bgfx::Attrib::Enum::Weight;
		break;
	case cd::VertexAttributeType::SourceVertexIndex:
		vertexAttribute = bgfx::Attrib::Enum::Indices;
		break;
	case cd::VertexAttributeType::MorphWeght:
		vertexAttribute = bgfx::Attrib::Enum::Weight;
		break;
	default:
		vertexAttribute = bgfx::Attrib::Enum::Count;
		break;
	}

	// Attribute Value
	switch (vertexAttributeLayout.attributeValueType)
	{
	case cd::AttributeValueType::Uint8:
		vertexAttributeValue = bgfx::AttribType::Enum::Uint8;
		break;
	case cd::AttributeValueType::Int16:
		vertexAttributeValue = bgfx::AttribType::Enum::Int16;
		break;
	case cd::AttributeValueType::Float:
		vertexAttributeValue = bgfx::AttribType::Enum::Float;
		break;
	default:
		vertexAttributeValue = bgfx::AttribType::Enum::Count;
		break;
	}

	assert(vertexAttribute != bgfx::Attrib::Enum::Count);
	assert(vertexAttributeValue != bgfx::AttribType::Enum::Count);
	outVertexLayout.add(vertexAttribute, vertexAttributeLayout.attributeCount, vertexAttributeValue, normalized);
}

}

namespace engine
{

// static
void VertexLayoutUtility::CreateVertexLayout(bgfx::VertexLayout& outVertexLayout, const std::vector<cd::VertexAttributeLayout>& vertexAttributes, bool debugPrint /* = false */)
{
	outVertexLayout.begin();
	for (const cd::VertexAttributeLayout& vertexAttributeLayout : vertexAttributes)
	{
		if (debugPrint)
		{
			CD_ENGINE_TRACE("\t\tVA: ({0}, {1}, {2})",
				nameof::nameof_enum(vertexAttributeLayout.vertexAttributeType),
				nameof::nameof_enum(vertexAttributeLayout.attributeValueType),
				vertexAttributeLayout.attributeCount);
		}
		ConvertVertexLayout(vertexAttributeLayout, outVertexLayout);
	}
	outVertexLayout.end();
}

// static
void VertexLayoutUtility::CreateVertexLayout(bgfx::VertexLayout& outVertexLayout, const cd::VertexAttributeLayout& vertexAttribute, bool debugPrint /* = false */)
{
	outVertexLayout.begin();
	if (debugPrint)
	{
		CD_ENGINE_TRACE("\t\tVA: ({0}, {1}, {2})",
			nameof::nameof_enum(vertexAttribute.vertexAttributeType),
			nameof::nameof_enum(vertexAttribute.attributeValueType),
			vertexAttribute.attributeCount);
	}
	ConvertVertexLayout(vertexAttribute, outVertexLayout);
	outVertexLayout.end();
}

}