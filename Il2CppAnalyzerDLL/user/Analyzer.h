#pragma once

#include <string>
#include <vector>

typedef int32_t HSCENE; // Scene handle

namespace Analyzer
{
	HSCENE GetCurrentSceneHandle(); // HSCENE
	std::string GetSceneName(HSCENE sceneHandle); // std::string
	bool IsSceneValid(HSCENE sceneHandle); // bool
	void* GetSceneGameObjects(HSCENE sceneHandle); // GameObject__Array*

	std::string GetGameObjectName(void* gameObject); // std::string
	std::string GetGameObjectTag(void* gameObject); // std::string
	std::vector<void*> GetGameObjectChildren(void* gameObject); // std::vector<GameObject*>
	std::vector<void*> GetGameObjectComponents(void* gameObject); // std::vector<Component_1*>
	
	Il2CppReflectionType* GetComponentType(void* component); // Il2CppReflectionType*
	std::string GetTypeName(void* type); // std::string
	
	const std::vector<const FieldInfo*> GetTypeFields(void* type); // std::vector<FieldInfo*>
	const std::string GetFieldName(const void* field); // std::string
	const std::string GetFieldNameFormatted(std::string name); // std::string
	const std::string GetFieldTypeName(const void* field); // std::string
	void* GetFieldValue(void* object, const void* field); // void*
	void SetFieldValue(void* object, const void* field, void* value); // void

	std::vector<void*>& GetDontDestroyOnLoadObjects(); // std::vector<Object_1*>&
	void AddDontDestroyOnLoadObject(void* object); // void
}