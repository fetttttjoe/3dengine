// Scene/Objects/Pyramid.h
#pragma once

#include "BaseObject.h"

class Pyramid : public BaseObject {
public:
    Pyramid();
    ~Pyramid() override = default;

    std::string GetTypeString() const override { return m_TypeString; }

protected:
    void BuildMeshData(std::vector<float>& vertices,
                       std::vector<unsigned int>& indices) override;
};
