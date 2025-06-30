
// =======================================================================
// File: src/Factories/SceneObjectFactory.h
// Description: A factory to decouple the Core application from concrete
//              scene object classes. Adheres to DIP.
// =======================================================================
#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>

// Forward-declare the interface, no need to include Interfaces.h here.
class ISceneObject;

class SceneObjectFactory {
public:
    using CreateObjectFunc = std::function<std::unique_ptr<ISceneObject>()>;

    void Register(const std::string& objectType, CreateObjectFunc func);
    std::unique_ptr<ISceneObject> Create(const std::string& objectType) const;

private:
    std::unordered_map<std::string, CreateObjectFunc> m_Registry;
};
