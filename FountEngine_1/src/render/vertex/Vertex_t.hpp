#pragma once
#include "../../math/CMath.hpp"

struct Vertex_t {
	CMath::Vector3_t vec3Position;
	CMath::Vector3_t vec3Normal;
	CMath::Vector2_t vec2Texcoord;
};