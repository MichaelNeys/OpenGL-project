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

std::vector<glm::vec3> BezierCurve::getPointsForwardDifferencing(int steps) const {
    std::vector<glm::vec3> points;
    if (steps <= 0) return points;

    float h = 1.0f / static_cast<float>(steps);
    float h2 = h * h;
    float h3 = h2 * h;

    // Bereken de coefficients van de matrixvorm van de cubic Bezier
    glm::vec3 a = -m_p0 + 3.0f * m_p1 - 3.0f * m_p2 + m_p3;
    glm::vec3 b = 3.0f * m_p0 - 6.0f * m_p1 + 3.0f * m_p2;
    glm::vec3 c = -3.0f * m_p0 + 3.0f * m_p1;
    glm::vec3 d = m_p0;

    // Initialiseer de startwaarden en de voorwaartse verschillen (forward differences)
    glm::vec3 p  = d;
    glm::vec3 dp = a * h3 + b * h2 + c * h;
    glm::vec3 d2p = 6.0f * a * h3 + 2.0f * b * h2;
    glm::vec3 d3p = 6.0f * a * h3;

    points.push_back(p);

    // De loop bevat nu ENKEL optellingen (dit is de essentie van forward differencing!)
    for (int i = 0; i < steps; i++) {
        p   += dp;
        dp  += d2p;
        d2p += d3p;
        points.push_back(p);
    }

    return points;
}