// Scene/Objects/Triangle.h
#pragma once

#include "BaseObject.h"

class Triangle : public BaseObject {
public:
    Triangle();
    ~Triangle() override = default;

    std::string GetTypeString() const override { return m_TypeString; }

protected:
    void BuildMeshData(std::vector<float>& vertices,
                       std::vector<unsigned int>& indices) override;
};
