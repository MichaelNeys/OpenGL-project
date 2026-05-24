#pragma once
#include <vector>
#include <glm/glm.hpp>

class BezierPath {
public:
    BezierPath();
    glm::vec3 getPoint(float t) const;
    float getTForDistance(float targetDistance) const;
    float getTotalLength() const { return totalCurveLength; }
    const std::vector<glm::vec3>& getControlPoints() const { return m_controlPoints; }

private:
    std::vector<glm::vec3> m_controlPoints;
    std::vector<float> arcLengthLUT;
    float totalCurveLength = 0.0f;
    void buildLUT(int lutResolution);
};