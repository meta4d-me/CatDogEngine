#pragma once

#include "Scene/VertexFormat.h"

#include <bgfx/bgfx.h>

namespace engine
{

class VertexLayoutUtility
{
public:
	static void CreateVertexLayout(bgfx::VertexLayout& outVertexLayout, const std::vector<cd::VertexFormat::VertexAttributeLayout>& vertexAttributes, bool debugPrint = false);
	static void CreateVertexLayout(bgfx::VertexLayout& outVertexLayout, const cd::VertexFormat::VertexAttributeLayout& vertexAttribute, bool debugPrint = false);
};

}