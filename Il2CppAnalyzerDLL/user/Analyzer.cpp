#include "pch-il2cpp.h"
#include "Analyzer.h"

#include <map>
#include <helpers.h>

#include "Output.h"

HSCENE Analyzer::GetCurrentSceneHandle() {
	HSCENE hScene = app::SceneManager_GetActiveScene(nullptr).m_Handle;
	if (!IsSceneValid(hScene)) {
		Output::Error("Scene handle is invalid!");
		return -1;
	}
	std::string sceneName = GetSceneName(hScene);
	return hScene;
}
std::string Analyzer::GetSceneName(HSCENE sceneHandle) {
	if (sceneHandle == -1) {
		Output::Error("Invalid scene handle!");
		return "";
	}
	app::Scene__Boxed bScene{};
	bScene.fields.m_Handle = sceneHandle;
	app::String* str = app::Scene_get_name(&bScene, nullptr);
	return il2cppi_to_string(str);
}
bool Analyzer::IsSceneValid(HSCENE sceneHandle) {
	return app::Scene_IsValidInternal(sceneHandle, nullptr);
}
void* Analyzer::GetSceneGameObjects(HSCENE sceneHandle) {
	if (sceneHandle == -1) {
		Output::Error("Invalid scene handle!");
		return nullptr;
	}
	app::Scene__Boxed bScene{};
	bScene.fields.m_Handle = sceneHandle;
	app::GameObject__Array* gameObjects = app::Scene_GetRootGameObjects(&bScene, nullptr);
	if (gameObjects == nullptr) {
		Output::Error("Failed to get scene game objects!");
		return nullptr;
	}
	return (void*)gameObjects;
}
std::string Analyzer::GetGameObjectName(void* gameObject) {
	app::String* str = app::Object_1_get_name((app::Object_1*)gameObject, nullptr);
	return il2cppi_to_string(str);
}
std::string Analyzer::GetGameObjectTag(void* gameObject) {
	app::String* str = app::GameObject_get_tag((app::GameObject*)gameObject, nullptr);
	return il2cppi_to_string(str);
}
std::vector<void*> Analyzer::GetGameObjectChildren(void* gameObject) {
	std::string rootName = GetGameObjectName(gameObject);
	std::vector<void*> children;
	app::Transform* rootTransform = app::GameObject_get_transform((app::GameObject*)gameObject, nullptr);
	if (rootTransform == nullptr) {
		Output::Error("Failed to get root transform!");
		return children;
	}
	int childCount = app::Transform_GetChildCount(rootTransform, nullptr);
	for (int i = 0; i < childCount; i++) {
		app::Transform* childTransform = app::Transform_GetChild(rootTransform, i, nullptr);
		if (childTransform == nullptr) {
			Output::Error("Failed to get child transform at index %i! (Parent: %s)", i, rootName.c_str());
			continue;
		}
		app::GameObject* child = app::Component_1_get_gameObject((app::Component_1*)childTransform, nullptr);
		children.push_back((void*)child);
	}
	return children;
}
std::vector<void*> Analyzer::GetGameObjectComponents(void* gameObject) {
	std::vector<void*> components;
	const Il2CppType* type = il2cpp_class_get_type((Il2CppClass*)*app::Component_1__TypeInfo);
	app::Type* componentType = (app::Type*)il2cpp_type_get_object(type);
	app::Component_1__Array* acomponents = app::GameObject_GetComponents((app::GameObject*)gameObject, componentType, nullptr);
	for (il2cpp_array_size_t i = 0; i < acomponents->max_length; i++) {
		components.push_back((void*)acomponents->vector[i]);
	}
	return components;
}
Il2CppReflectionType* Analyzer::GetComponentType(void* component) {
	Il2CppClass* klass = il2cpp_object_get_class((Il2CppObject*)component);
	if (klass == nullptr) {
		Output::Error("Failed to get component class!");
		return nullptr;
	}
	const Il2CppType* type = il2cpp_class_get_type(klass);
	return (Il2CppReflectionType*)il2cpp_type_get_object(type);
}
std::string Analyzer::GetTypeName(void* type) {
	return il2cpp_type_get_name(((Il2CppReflectionType*)type)->type);
}
const std::vector<const FieldInfo*> Analyzer::GetTypeFields(void* type) {
	std::vector<const FieldInfo*> fields;
	Il2CppClass* klass = il2cpp_class_from_system_type((Il2CppReflectionType*)type);
	if (klass == nullptr) {
		Output::Error("Failed to get class from type!");
		return fields;
	}
	void* iter = nullptr;
	const FieldInfo* field = nullptr;
	while ((field = il2cpp_class_get_fields(klass, &iter)) != nullptr) {
		fields.push_back(field);
	}
	return fields;
}
const std::string Analyzer::GetFieldName(const void* field) {
	return ((const FieldInfo*)field)->name;
}
const std::string Analyzer::GetFieldNameFormatted(std::string name) {
	if (name.substr(0, 2) == "m_") {
		name = name.substr(2);
	}
	std::string formatted("");
	std::string word("");
	for (char c : name) {
		if (c >= 'A' && c <= 'Z') {
			if (!word.empty()) {
				formatted += word + " ";
				word.clear();
			}
			word += c;
		}
		else {
			if (word.empty() && c >= 'a' && c <= 'z') {
				c -= 32;
			}
			word += c;
		}
	}
	if (!word.empty()) {
		formatted += word;
	}
	return formatted;
}
const std::string Analyzer::GetFieldTypeName(const void* field) {
	const Il2CppType* type = il2cpp_field_get_type((FieldInfo*)field);
	return il2cpp_type_get_name(type);
}
void* Analyzer::GetFieldValue(void* object, const void* field) {
	void* value = nullptr;
	il2cpp_field_get_value((Il2CppObject*)object, (FieldInfo*)field, &value);
	return value;
}
void Analyzer::SetFieldValue(void* object, const void* field, void* value) {
	il2cpp_field_set_value((Il2CppObject*)object, (FieldInfo*)field, value);
}

std::vector<void*> dontDestroyOnLoadObjects;
std::vector<void*>& Analyzer::GetDontDestroyOnLoadObjects() {
	return dontDestroyOnLoadObjects;
}
void Analyzer::AddDontDestroyOnLoadObject(void* object) {
	dontDestroyOnLoadObjects.push_back(object);
}
