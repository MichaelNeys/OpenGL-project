#pragma once
#include <glm/glm.hpp>
#include <vector>

class BezierCurve {
public:
    // Een cubic curve heeft exact 4 controlepunten
    BezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

    // Evalueert de curve op lokale t [0, 1] met een snelle formule
    glm::vec3 getPoint(float t) const; 

    std::vector<glm::vec3> getPointsForwardDifferencing(int steps) const;

private:
    glm::vec3 m_p0, m_p1, m_p2, m_p3;
};