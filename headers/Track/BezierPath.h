// BezierPath.h
#pragma once
#include "BezierCurve.h"
#include <vector>

class BezierPath {
public:
    // Je geeft de punten nu mee, ze staan niet meer vastgelijmd in de klasse!
    BezierPath(const std::vector<glm::vec3>& controlPoints);

    glm::vec3 getPoint(float t) const;
    float getTForDistance(float targetDistance) const;
    float getTotalLength() const { return totalCurveLength; }
    std::vector<glm::vec3> generateVisualPath(int stepsPerSegment) const;

private:
    void buildLUT(int resolution);

    std::vector<BezierCurve> m_curves; // Je parcours opgeknipt in stukjes
    std::vector<float> arcLengthLUT;
    float totalCurveLength;
};