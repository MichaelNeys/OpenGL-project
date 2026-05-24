#include "BezierPath.h"

BezierPath::BezierPath() {
    // 45 hardcode control points
    m_controlPoints = {
        glm::vec3(15.0f, 0.0f, -10.0f),
        glm::vec3(13.0f, -1.0f, -10.0f),
        glm::vec3(10.0f, 0.8f, -10.0f),
        glm::vec3(5.5f, 0.0f, -10.0f),
        glm::vec3(9.0f, 0.65f,-15.0f),
        glm::vec3(1.0f, 0.0f, -15.0f),
        glm::vec3(0.0f, 0.2f, -20.0f),
        glm::vec3(-0.4f, 0.6f, -20.0f),
        glm::vec3(-1.5f, 2.0f, -13.5f),
        glm::vec3(-4.0f, 1.4f, -9.0f),
        glm::vec3(-6.0f, 0.7f, -9.2f),
        glm::vec3(-7.0f, 0.7f, -9.5f),
        glm::vec3(-8.5f, -1.5f, -10.0f),
        glm::vec3(-7.0f, 0.7f, -9.5f),
        glm::vec3(-12.0f,-1.6f, -10.5f),
        glm::vec3(-20.0f,-0.9f, -6.0f),
        glm::vec3(-18.0f, 0.0f, -5.0f),
        glm::vec3(-11.0f, 0.1f, -6.5f),
        glm::vec3(-13.0f, 0.0f, -15.0f),
        glm::vec3(-11.0f, 0.5f, -16.0f),
        glm::vec3(-13.0f, 0.5f, -17.5f),
        glm::vec3(-13.0f,-0.7f, -18.0f),
        glm::vec3(-11.5f,-1.5f, -20.0f),
        glm::vec3(5.1f,-1.5f, -35.0f),
        glm::vec3(5.8f,-1.8f, -38.0f),
        glm::vec3(-35.0f, 0.4f, -36.0f),
        glm::vec3(-30.0f, 1.0f, -14.0f),
        glm::vec3(-19.0f, 0.5f, -9.0f),
        glm::vec3(-14.0f, -0.5f, -18.0f),
        glm::vec3(-1.0f, 0.0f, -18.0f),
        glm::vec3(0.0f, 0.0f, -18.0f),
        glm::vec3(1.0f, 0.0f, -18.0f),
        glm::vec3(4.0f, 0.0f, -18.0f),
        glm::vec3(7.0f, 0.5f, -18.0f),
        glm::vec3(10.0f, 1.0f, -18.0f),
        glm::vec3(20.0f, 0.0f, -22.5f),
        glm::vec3(19.0f,-0.5f, -25.5f),
        glm::vec3(19.0f,-0.5f, -20.0f),
        glm::vec3(18.0f,-2.0f, -18.5f),
        glm::vec3(17.0f,-3.0f, -14.0f),
        glm::vec3(17.0f,-0.7f, -14.25f),
        glm::vec3(17.0f, 0.7f, -14.5f),
        glm::vec3(17.0f, 1.0f, -11.0f),
        glm::vec3(17.0f, 1.0f, -10.0f),
        glm::vec3(15.0f, 0.0f, -10.0f),
    };
    buildLUT(1000);
}

glm::vec3 BezierPath::getPoint(float t) const {
    if (m_controlPoints.empty()) return glm::vec3(0.0f);
    if (m_controlPoints.size() == 1) return m_controlPoints[0];

    std::vector<glm::vec3> temp = m_controlPoints;
    int n = temp.size() - 1;
    for (int k = 1; k <= n; k++)
        for (int i = 0; i <= n - k; i++)
            temp[i] = (1.0f - t) * temp[i] + t * temp[i + 1];
    return temp[0];
}

void BezierPath::buildLUT(int lutResolution) {
    arcLengthLUT.clear();
    arcLengthLUT.push_back(0.0f);
    glm::vec3 prevPoint = getPoint(0.0f);
    float currentLen = 0.0f;
    for (int i = 1; i <= lutResolution; i++) {
        float t = (float)i / (float)lutResolution;
        glm::vec3 pt = getPoint(t);
        currentLen += glm::length(pt - prevPoint);
        arcLengthLUT.push_back(currentLen);
        prevPoint = pt;
    }
    totalCurveLength = currentLen;
}

float BezierPath::getTForDistance(float targetDistance) const {
     if (targetDistance <= 0.0f) return 0.0f;
    if (targetDistance >= totalCurveLength) return 1.0f;
    for (size_t i = 0; i < arcLengthLUT.size() - 1; i++) {
        if (targetDistance >= arcLengthLUT[i] && targetDistance <= arcLengthLUT[i+1]) {
            float segLen = arcLengthLUT[i+1] - arcLengthLUT[i];
            float segFrac = (targetDistance - arcLengthLUT[i]) / segLen;
            float t0 = (float)i / (float)(arcLengthLUT.size() - 1);
            float t1 = (float)(i + 1) / (float)(arcLengthLUT.size() - 1);
            return t0 + segFrac * (t1 - t0);
        }
    }
    return 1.0f;
}