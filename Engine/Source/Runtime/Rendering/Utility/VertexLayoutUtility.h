#pragma once

#include "Scene/VertexAttribute.h"

#include <bgfx/bgfx.h>

#include <vector>

namespace engine
{

class VertexLayoutUtility
{
public:
	static void CreateVertexLayout(bgfx::VertexLayout& outVertexLayout, const std::vector<cd::VertexAttributeLayout>& vertexAttributes, bool debugPrint = false);
	static void CreateVertexLayout(bgfx::VertexLayout& outVertexLayout, const cd::VertexAttributeLayout& vertexAttribute, bool debugPrint = false);
};

}