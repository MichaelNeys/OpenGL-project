#include "Track/BezierCurve.h"
#include <cmath>

/**
 * @brief Construeert een nieuw Cubic Bézier-segment met 4 controlepunten.
 * 
 * @param p0 startpunt (op de curve)
 * @param p1 eerste controlepunt (hulp)
 * @param p2 tweede controlepunt (hulp)
 * @param p3 eindpunt (op de curve)
 */
BezierCurve::BezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
    : m_p0(p0), m_p1(p1), m_p2(p2), m_p3(p3) {}


/**
 * @brief Berekent de positie op de Bézier-curve voor een gegeven parameter t
 * 
 * Maakt gebruik van de expliciete/analytische polynoomformule voor een derdegraads 
 * Bézier-curve (efficiënt voor willekeurige punten op te vragen)
 * 
 * @param t curve-parameter (verwachte waarde tussen 0.0f en 1.0f)
 * @return glm::vec3 berekende 3D-positie op de curve
 */
glm::vec3 BezierCurve::getPoint(float t) const {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 p = uuu * m_p0;
    p += 3.0f * uu * t * m_p1;
    p += 3.0f * u * tt * m_p2;
    p += ttt * m_p3;

    return p;
}

/**
 * @brief Berekent een reeks punten op de Bézier-curve met behulp van forward differencing
 * 
 * curve in een iteratieve matrixvorm, binnenste lus bevat uitsluitend
 * vector sommen (sneller)
 * 
 * @param steps aantal punten dat gegenereerd moet worden
 * @return std::vector<glm::vec3> een vector met de gegenereerde punten
 */
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

    // init start- en voorwaartse verschillen
    glm::vec3 p  = d;
    glm::vec3 dp = a * h3 + b * h2 + c * h;
    glm::vec3 d2p = 6.0f * a * h3 + 2.0f * b * h2;
    glm::vec3 d3p = 6.0f * a * h3;

    points.push_back(p);

    // enkel optellingen
    for (int i = 0; i < steps; i++) {
        p   += dp;
        dp  += d2p;
        d2p += d3p;
        points.push_back(p);
    }

    return points;
}