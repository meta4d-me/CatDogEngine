#include "Rendering/Utility/VertexLayoutUtility.h"

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
constexpr uint32_t MAX_COLOR_COUNT = 4;

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
constexpr uint32_t MAX_UV_COUNT = 8;

std::string VertexAttributeTypeToString(const cd::VertexAttributeType& attribType)
{
	switch (attribType)
	{
	case cd::VertexAttributeType::Position:
		return "Position";
	case cd::VertexAttributeType::Normal:
		return "Normal";
	case cd::VertexAttributeType::Tangent:
		return "Tangent";
	case cd::VertexAttributeType::Bitangent:
		return "Bitangent";
	case cd::VertexAttributeType::UV:
		return "UV";
	case cd::VertexAttributeType::Color:
		return "Color";
	default:
		return "Invalid Attribute Type!";
	}
}

std::string AttributeValueTypeToString(const cd::AttributeValueType& attribValueType)
{
	switch (attribValueType)
	{
	case cd::AttributeValueType::Float:
		return "Float";
	case cd::AttributeValueType::Uint8:
		return "Uint8";
	default:
		return "Invalid Attribute Value Type!";
	}
}

void ConvertVertexLayout(const cd::VertexAttributeLayout& vertexAttributeLayout, bgfx::VertexLayout& outVertexLayout)
{
	bgfx::Attrib::Enum vertexAttrib = bgfx::Attrib::Enum::Count;
	bgfx::AttribType::Enum vertexAttribValue = bgfx::AttribType::Enum::Count;
	bool normalized = false;

	// Attribute Type
	switch (vertexAttributeLayout.vertexAttributeType)
	{
	case cd::VertexAttributeType::Position:
		vertexAttrib = bgfx::Attrib::Enum::Position;
		break;
	case cd::VertexAttributeType::Normal:
		vertexAttrib = bgfx::Attrib::Enum::Normal;
		normalized = true;
		break;
	case cd::VertexAttributeType::Tangent:
		vertexAttrib = bgfx::Attrib::Enum::Tangent;
		break;
	case cd::VertexAttributeType::Bitangent:
		vertexAttrib = bgfx::Attrib::Enum::Bitangent;
		break;
	case cd::VertexAttributeType::UV:
		vertexAttrib = bgfx::Attrib::Enum::Count;
		for (const bgfx::Attrib::Enum& textCoord : AllAttribUVTypes)
		{
			if (!outVertexLayout.has(textCoord))
			{
				vertexAttrib = textCoord;
				break;
			}
		}
		break;
	case cd::VertexAttributeType::Color:
		vertexAttrib = bgfx::Attrib::Enum::Count;
		for (const bgfx::Attrib::Enum& color : AllAttribColorTypes)
		{
			if (!outVertexLayout.has(color))
			{
				vertexAttrib = color;
				break;
			}
		}
		break;
	default:
		vertexAttrib = bgfx::Attrib::Enum::Count;
		break;
	}

	// Attribute Value
	switch (vertexAttributeLayout.attributeValueType)
	{
	case cd::AttributeValueType::Uint8:
		vertexAttribValue = bgfx::AttribType::Enum::Uint8;
		break;
	case cd::AttributeValueType::Float:
		vertexAttribValue = bgfx::AttribType::Enum::Float;
		break;
	default:
		vertexAttribValue = bgfx::AttribType::Enum::Count;
		break;
	}

	assert(vertexAttrib != bgfx::Attrib::Enum::Count);
	assert(vertexAttribValue != bgfx::AttribType::Enum::Count);
	outVertexLayout.add(vertexAttrib, vertexAttributeLayout.attributeCount, vertexAttribValue, normalized);
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
			printf("\t\tVA: (%s, %s, %d)\n",
				VertexAttributeTypeToString(vertexAttributeLayout.vertexAttributeType).c_str(),
				AttributeValueTypeToString(vertexAttributeLayout.attributeValueType).c_str(),
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
		printf("\t\tVA: (%s, %s, %d)\n",
			VertexAttributeTypeToString(vertexAttribute.vertexAttributeType).c_str(),
			AttributeValueTypeToString(vertexAttribute.attributeValueType).c_str(), 
			vertexAttribute.attributeCount);
	}
	ConvertVertexLayout(vertexAttribute, outVertexLayout);
	outVertexLayout.end();
}

}