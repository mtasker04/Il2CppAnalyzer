#include "pch-il2cpp.h"
#include "AnalyzerGUI.h"

#include <Windows.h>
#include <vector>
#include <string>
#include <imgui.h>
#include <helpers.h>

#include "Analyzer.h"
#include "Output.h"

static ImFont* g_regularFont = nullptr;
static ImFont* g_semiBoldFont = nullptr;
static ImFont* g_boldFont = nullptr;

static const ImVec4 Node_Selected = ImVec4(0.17254f, 0.36470f, 0.52941f, 1.00f);

static int lastId = 0;
struct HierachyNode {
	int id;
	app::GameObject* gameObject;
	std::string name;
	int idx;
	std::vector<HierachyNode> children;
	bool loaded = false;

	std::string GetFullName() {
		std::string fullname;
		fullname = name == "" ? "[Unnamed]" : name;
		if (idx > 0) {
			fullname += " [" + std::to_string(idx) + "]";
		}
		return fullname;
	}
};

static std::vector<HierachyNode> Scenes;
static HierachyNode* SelectedNode = nullptr;

static int _GetNodeIdx(std::vector<HierachyNode>& nodes, std::string name) {
	int idx = 0;
	for (HierachyNode& node : nodes) {
		if (node.name == name) {
			idx++;
		}
	}
	return idx;
}
static std::vector<HierachyNode> _GetGameObjectHierachy(app::GameObject* gameObject) {
	std::vector<void*> children = Analyzer::GetGameObjectChildren((void*)gameObject);
	std::vector<HierachyNode> heirachy;
	for (void* child : children) {
		HierachyNode node;
		node.id = lastId++;
		node.gameObject = (app::GameObject*)child;
		node.name = Analyzer::GetGameObjectName(child);
		node.idx = _GetNodeIdx(heirachy, node.name);
		heirachy.push_back(node);
	}
	return heirachy;
}
static HierachyNode _GetDontDestroyOnLoadHierachy() {
	lastId = 0;
	HierachyNode root;
	root.name = "DontDestroyOnLoad";
	root.id = lastId++;
	root.gameObject = nullptr;
	root.loaded = true;
	root.idx = 0;
	std::vector<void*> objects = Analyzer::GetDontDestroyOnLoadObjects();
	for (void* object : objects) {
		HierachyNode node;
		node.id = lastId++;
		node.gameObject = (app::GameObject*)object;
		node.name = Analyzer::GetGameObjectName(object);
		root.children.push_back(node);
	}
	return root;
}
static HierachyNode _GetSceneHierachy() {
	lastId = 0;
	HierachyNode root;
	HSCENE currentSceneHandle = Analyzer::GetCurrentSceneHandle();
	root.name = "Scene";
	root.id = lastId++;
	root.gameObject = nullptr;
	root.loaded = true;
	root.idx = 0;
	app::GameObject__Array* gameObjects = (app::GameObject__Array*)Analyzer::GetSceneGameObjects(currentSceneHandle);
	for (il2cpp_array_size_t i = 0; i < gameObjects->max_length; i++) {
		app::GameObject* gameObject = gameObjects->vector[i];
		HierachyNode node;
		node.id = lastId++;
		node.gameObject = gameObject;
		node.name = Analyzer::GetGameObjectName((void*)gameObject);
		root.children.push_back(node);
	}
	return root;
}
static void _RenderHierachy(HierachyNode& root) {
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (SelectedNode == &root) {
		flags |= ImGuiTreeNodeFlags_Selected;
		ImGui::PushStyleColor(ImGuiCol_Header, Node_Selected);
	}
	if (root.id == 0) {
		flags |= ImGuiTreeNodeFlags_DefaultOpen;
		ImGui::PushFont(g_boldFont);
	}
	if (root.id == 0 || root.children.size() == 0) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool open = ImGui::TreeNodeEx(root.GetFullName().c_str(), flags);
	if (SelectedNode == &root) {
		ImGui::PopStyleColor();
	}
	if (root.id == 0) {
		ImGui::PopFont();
	}
	if (ImGui::IsItemClicked() && root.id != 0) {
		SelectedNode = &root;
	}

	if (open) {
		if (!root.loaded) {
			root.children = _GetGameObjectHierachy(root.gameObject);
			root.loaded = true;
		}
		for (HierachyNode& child : root.children) {
			_RenderHierachy(child);
		}
		if (!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
			ImGui::TreePop();
		}
	}
}
static int fieldId = 0;
static void _CreateFieldInput(app::Component_1* component, const FieldInfo* fieldInfo) {
	std::string fieldName = Analyzer::GetFieldName((void*)fieldInfo);
	fieldName = Analyzer::GetFieldNameFormatted(fieldName);
	std::string fieldTypeName = Analyzer::GetFieldTypeName((void*)fieldInfo);
	void* fieldValue = Analyzer::GetFieldValue((void*)component, fieldInfo);

	float labelWidth = 150.0f; // Set the desired label width
    ImGui::Text("%s", fieldName.c_str());
    ImGui::SameLine();
    ImGui::SetCursorPosX(labelWidth);

	std::string id = std::to_string(fieldId++);
	const char* nolabel = ("##" + std::string(id)).c_str();

    if (fieldTypeName == "System.String") {
        Il2CppString* str = (Il2CppString*)fieldValue;
        if (str->length > 512) {
            ImGui::Text("String too long to display.");
            return;
        }
        std::string value = il2cppi_to_string(str);
        char buffer[512];
        strcpy_s(buffer, value.c_str());
		if (ImGui::InputText(nolabel, buffer, sizeof(buffer))) {
            Analyzer::SetFieldValue((void*)component, fieldInfo, buffer);
        }
    }
    else if (fieldTypeName == "System.Int32") {
        if (ImGui::InputInt(nolabel, (int*)&fieldValue)) {
            Analyzer::SetFieldValue((void*)component, fieldInfo, (int*)&fieldValue);
        }
    }
    else if (fieldTypeName == "System.Single") {
        if (ImGui::InputFloat(nolabel, (float*)&fieldValue)) {
            Analyzer::SetFieldValue((void*)component, fieldInfo, (float*)&fieldValue);
        }
    }
    else if (fieldTypeName == "System.Boolean") {
        if (ImGui::Checkbox(nolabel, (bool*)&fieldValue)) {
            Analyzer::SetFieldValue((void*)component, fieldInfo, (bool*)&fieldValue);
        }
    }
    else if (fieldTypeName == "UnityEngine.Vector3") {
        app::Vector3* value = (app::Vector3*)fieldValue;
        if (ImGui::InputFloat3(nolabel, &value->x)) {
            Analyzer::SetFieldValue((void*)component, fieldInfo, value);
        }
    }
    else {
        ImGui::Text("Unsupported field type: %s", fieldTypeName.c_str());
    }
}
static void _RenderComponents(app::GameObject* gameObject) {
	std::vector<void*> components = Analyzer::GetGameObjectComponents((void*)gameObject);
	for (void* component : components) {
		app::Component_1* comp = (app::Component_1*)component;
		Il2CppReflectionType* compType = Analyzer::GetComponentType(component);
		std::string compName = Analyzer::GetTypeName((void*)compType);
		ImGui::Text("%s", compName.c_str());
		std::vector<const FieldInfo*> fields = Analyzer::GetTypeFields((void*)compType);
		for (const auto& field : fields) {
			_CreateFieldInput(comp, field);
		}
		ImGui::Separator();
	}
}
static void _RenderInspector(HierachyNode* node) {
	if (node == nullptr) {
		ImGui::Text("No object selected.");
		return;
	}
	app::GameObject* gameObject = node->gameObject;
	std::string objectName = Analyzer::GetGameObjectName((void*)gameObject);
	ImGui::Text("Name: %s", objectName.c_str());
	std::string objectTag = Analyzer::GetGameObjectTag((void*)gameObject);
	ImGui::Text("Tag: %s", objectTag.c_str());
	ImGui::Separator();
	_RenderComponents(gameObject);
}

static void _SetStyle() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Set the style colors
	colors[ImGuiCol_Text] = ImVec4(0.77647f, 0.77647f, 0.77647f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.41176f, 0.41176f, 0.41176f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.17647f, 0.17647f, 0.17647f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.09803f, 0.09803f, 0.09803f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.16470f, 0.16470f, 0.16470f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16470f, 0.16470f, 0.16470f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f); // todo
	colors[ImGuiCol_TitleBg] = ImVec4(0.15686f, 0.15686f, 0.15686f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.15686f, 0.15686f, 0.15686f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15686f, 0.15686f, 0.15686f, 0.2f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20784f, 0.20784f, 0.20784f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.37254f, 0.37254f, 0.37254f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.00f); // guess
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f); // guess
	colors[ImGuiCol_CheckMark] = ImVec4(0.80392f, 0.80392f, 0.80392f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f); // todo
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.37f, 0.37f, 1.00f); // todo
	colors[ImGuiCol_Button] = ImVec4(0.34509f, 0.34509f, 0.34509f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f); // guess
	colors[ImGuiCol_ButtonActive] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f); // guess
	colors[ImGuiCol_Header] = ImVec4(0.21960f, 0.21960f, 0.21960f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.21960f, 0.21960f, 0.21960f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.21960f, 0.21960f, 0.21960f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.10980f, 0.10980f, 0.10980f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10980f, 0.10980f, 0.10980f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10980f, 0.10980f, 0.10980f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.15686f, 0.15686f, 0.15686f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f); // guess
	colors[ImGuiCol_TabActive] = ImVec4(0.23529f, 0.23529f, 0.23529f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.15686f, 0.15686f, 0.15686f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.23529f, 0.23529f, 0.23529f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f); // todo
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f); // todo
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f); // todo
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f); // todo
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f); // todo
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f); // todo
	colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f); // todo
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f); // todo
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f); // todo
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f); // todo

	// Set the style variables
	style.WindowRounding = 3.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowPadding = ImVec2(0.0f, 0.0f);
	style.FrameRounding = 2.3f;
	style.ScrollbarRounding = 1.0f;
	style.GrabRounding = 2.3f;
	style.TabRounding = 2.3f;

	// Set the tree node style variables
	style.IndentSpacing = 15.0f;
	style.FramePadding = ImVec2(0.0f, 0.0f);
	style.ItemSpacing = ImVec2(0.0f, 0.0f);
}
static void _LoadFonts() {
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	g_regularFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	g_semiBoldFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisb.ttf", 18.0f);
	g_boldFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 18.0f);
	io.FontDefault = g_regularFont;
}
void AnalyzerGUI::Initialize() {
	_SetStyle();
	_LoadFonts();
	Scenes.push_back(_GetSceneHierachy());
	Scenes.push_back(_GetDontDestroyOnLoadHierachy());
}
void AnalyzerGUI::Render() {
	if (ImGui::Begin("Hierachy")) {
		for (size_t i = 0; i < Scenes.size(); i++) {
			_RenderHierachy(Scenes[i]);
		}
	}
	ImGui::End();

	if (ImGui::Begin("Inspector")) {
		_RenderInspector(SelectedNode);
	}
	ImGui::End();
}
void AnalyzerGUI::RefreshHierachy() {
	Scenes.clear();
	Scenes.push_back(_GetSceneHierachy());
	Scenes.push_back(_GetDontDestroyOnLoadHierachy());
}