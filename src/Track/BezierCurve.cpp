#include "Track/BezierCurve.h"
#include <cmath>

BezierCurve::BezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
    : m_p0(p0), m_p1(p1), m_p2(p2), m_p3(p3) {}

glm::vec3 BezierCurve::getPoint(float t) const {
    // Analytische formule voor een Cubic Bézier curve (veel sneller voor realtime rendering)
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 p = uuu * m_p0;               // Term 1
    p += 3.0f * uu * t * m_p1;              // Term 2
    p += 3.0f * u * tt * m_p2;              // Term 3
    p += ttt * m_p3;                        // Term 4

    return p;
}