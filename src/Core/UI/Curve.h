#pragma once

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

// A point on the curve editor
struct CurvePoint {
    glm::vec2 pos;
};

// Represents a 1D curve defined by a series of points
class Curve {
public:
    // Adds a new control point and sorts them by their x-coordinate
    void AddPoint(const glm::vec2& pos) {
        m_Points.push_back({pos});
        SortPoints();
    }

    // Evaluates the curve at a given x-coordinate using linear interpolation
    float Evaluate(float x) const {
        if (m_Points.empty()) return 0.0f;
        if (x <= m_Points.front().pos.x) return m_Points.front().pos.y;
        if (x >= m_Points.back().pos.x) return m_Points.back().pos.y;

        auto it = std::upper_bound(m_Points.begin(), m_Points.end(), x, 
            [](float val, const CurvePoint& p) {
            return val < p.pos.x;
        });

        auto p2 = it;
        auto p1 = std::prev(it);

        float t = (x - p1->pos.x) / (p2->pos.x - p1->pos.x);
        return glm::mix(p1->pos.y, p2->pos.y, t);
    }

    std::vector<CurvePoint>& GetPoints() { return m_Points; }
    
    // Sorts points by their x-coordinate. Must be called after manual modification.
    void SortPoints() {
        std::sort(m_Points.begin(), m_Points.end(), 
            [](const CurvePoint& a, const CurvePoint& b) {
            return a.pos.x < b.pos.x;
        });
    }

private:
    std::vector<CurvePoint> m_Points;
};