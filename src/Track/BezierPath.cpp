#include "Track/BezierPath.h"
#include <iostream>
#include <algorithm>

/**
 * @brief constructor
 * 
 * creeert segmenten vd samengestelde curve op basis van controlepunten
 * init LUT voor booglengte
 * 
 * @param controlPoints vector controlepunten (aantal = 3n+1)
 */
BezierPath::BezierPath(const std::vector<glm::vec3>& controlPoints) {
    if (controlPoints.size() < 4 || (controlPoints.size() - 1) % 3 != 0) {
        std::cerr << "Fout: Aantal controlepunten moet voldoen aan (3n + 1) voor Cubic Bézier Splines! (bv. 4, 7, 10, 13...)" << std::endl;
        return;
    }

    // Hak de grote lijst met punten op in Cubic Bézier segmenten (elk 4 punten)
    for (size_t i = 0; i < controlPoints.size() - 1; i += 3) {
        m_curves.push_back(BezierCurve(
            controlPoints[i],
            controlPoints[i + 1],
            controlPoints[i + 2],
            controlPoints[i + 3]
        ));
    }

    // Bouw de LUT op basis van alle segmenten gecombineerd
    buildLUT(1000); 
}

/**
 * @brief berekent positie op curve voor t tussen 0 en 1
 * 
 * eerst welk deelsegment, dan lokale t voor dat segment
 * 
 * @param t parameter tussen 0 (begin) en 1 (einde)
 * @return glm::vec3 positie op curve
 */
glm::vec3 BezierPath::getPoint(float t) const {
    if (m_curves.empty()) return glm::vec3(0.0f);
    
    // t inklemmen tussen 0 en 1
    t = glm::clamp(t, 0.0f, 1.0f);

    // Bepaal in welk Bézier-segment we ons bevinden
    float numCurves = static_cast<float>(m_curves.size());
    float scaledT = t * numCurves;
    size_t curveIndex = static_cast<size_t>(scaledT);

    if (curveIndex >= m_curves.size()) {
        curveIndex = m_curves.size() - 1;
    }

    // Bereken de lokale t [0, 1] voor dat specifieke segment
    float localT = scaledT - static_cast<float>(curveIndex);

    return m_curves[curveIndex].getPoint(localT);
}

/**
 * @brief build LUT voor booglengte aan t te linken
 * 
 * zo creeeren we een constante snelheid over de curve
 * hoe de controlepunten ook verspreid liggen
 * 
 * @param resolution aantal samples dat gebruikt wordt om de lengte te benaderen
 */
void BezierPath::buildLUT(int resolution) {
    arcLengthLUT.clear();
    arcLengthLUT.reserve(resolution + 1);
    arcLengthLUT.push_back(0.0f);

    glm::vec3 prevPoint = getPoint(0.0f);
    float currentLen = 0.0f;

    for (int i = 1; i <= resolution; i++) {
        float t = static_cast<float>(i) / static_cast<float>(resolution);
        glm::vec3 pt = getPoint(t);
        
        currentLen += glm::length(pt - prevPoint);
        arcLengthLUT.push_back(currentLen);
        
        prevPoint = pt;
    }

    totalCurveLength = currentLen;
}

/**
 * @brief Zette een gewenste fysieke afstand om naar de bijbehorende t-parameter [0.0, 1.0].
 * 
 * maakt gebruik van std::lower_bound op  LUT en een lineaire interpolatie
 * voor een nauwkeurige en snelle t-waarde
 * 
 * @param targetDistance afstand vanaf begin vd curve
 * @return float t parameter bij die afstand
 */
float BezierPath::getTForDistance(float targetDistance) const {
    if (targetDistance <= 0.0f) return 0.0f;
    if (targetDistance >= totalCurveLength) return 1.0f;

    // Binary Search op arcLengthLUT
    auto iterator = std::lower_bound(arcLengthLUT.begin(), arcLengthLUT.end(), targetDistance);
    size_t index = std::distance(arcLengthLUT.begin(), iterator);

    if (index == 0) return 0.0f;

    size_t i = index - 1;
    float segLen = arcLengthLUT[i + 1] - arcLengthLUT[i];
    float segFrac = (targetDistance - arcLengthLUT[i]) / segLen;

    float t0 = static_cast<float>(i) / static_cast<float>(arcLengthLUT.size() - 1);
    float t1 = static_cast<float>(i + 1) / static_cast<float>(arcLengthLUT.size() - 1);

    return t0 + segFrac * (t1 - t0);
}

/**
 * @brief genereert punten die het pad voorstellen
 * 
 * maakt gebruik van Forward Differencing voor snelle berekening van punten per segment
 * 
 * @param stepsPerSegment punten per segment
 * @return std::vector<glm::vec3> array met punten die het pad voorstellen
 */
std::vector<glm::vec3> BezierPath::generateVisualPath(int stepsPerSegment) const {
    std::vector<glm::vec3> totalPath;
    
    for (const auto& curve : m_curves) {
        std::vector<glm::vec3> segmentPoints = curve.getPointsForwardDifferencing(stepsPerSegment);
        totalPath.insert(totalPath.end(), segmentPoints.begin(), segmentPoints.end());
    }
    
    return totalPath;
}