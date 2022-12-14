#pragma once

#include "glm/ext/quaternion_float.hpp"
#include "glm/vec3.hpp"

namespace SHV {
struct AABBox {
    glm::vec3 min = {};
    glm::vec3 max = {};
    bool valid = false;
    AABBox(){};
    AABBox(glm::vec3 min, glm::vec3 max) : min(min), max(max), valid(true){};

    AABBox GetAABB(const glm::mat4& m);
};
}  // namespace SHV