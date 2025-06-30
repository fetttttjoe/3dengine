
// =======================================================================
// File: src/Factories/SceneObjectFactory.cpp
// =======================================================================
#include "Factories/SceneObjectFactory.h"
#include "Interfaces.h" // Include full definition for std::unique_ptr
#include <iostream>

void SceneObjectFactory::Register(const std::string& objectType, CreateObjectFunc func) {
    m_Registry[objectType] = func;
}

std::unique_ptr<ISceneObject> SceneObjectFactory::Create(const std::string& objectType) const {
    auto it = m_Registry.find(objectType);
    if (it != m_Registry.end()) {
        return it->second();
    }
    std::cerr << "Error: Factory cannot create unknown object type '" << objectType << "'." << std::endl;
    return nullptr;
}