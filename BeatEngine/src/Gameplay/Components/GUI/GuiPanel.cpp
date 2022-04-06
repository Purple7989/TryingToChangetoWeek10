#include "GuiPanel.h"
#include "Application/Application.h"
#include "Graphics/GuiBatcher.h"
#include "Gameplay/GameObject.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

GuiPanel::GuiPanel() :
	_borderRadius(-1),
	_color(glm::vec4(1.0f)),
	_texture(nullptr),
	_transform(nullptr)
{ }

GuiPanel::GuiPanel(float perX, float perY, float width, float height) :
	_borderRadius(-1),
	_color(glm::vec4(1.0f)),
	_texture(nullptr),
	_transform(nullptr),

	_percentOfScreenX(perX),
	_percentOfScreenY(perY),
	_width(width),
	_height(height)
{ }

GuiPanel::~GuiPanel() = default;

void GuiPanel::SetColor(const glm::vec4 & color) {
	_color = color;
}

const glm::vec4& GuiPanel::GetColor() const {
	return _color;
}

int GuiPanel::GetBorderRadius() const {
	return _borderRadius;
}

void GuiPanel::SetBorderRadius(int value) {
	_borderRadius = value;
}

Texture2D::Sptr GuiPanel::GetTexture() const {
	return _texture;
}

void GuiPanel::SetTexture(const Texture2D::Sptr & value) {
	_texture = value;
}

void GuiPanel::Awake() {

	Application& app = Application::Get();
	glm::vec2 windowSize = app.GetWindowSize();

	//Store the propotions of the GUI element based on a native 1920x1080 screen size
	_proportionX = _width / 1920;
	_proportionY = _height / 1080;


	_transform = GetComponent<RectTransform>();
	if (_transform == nullptr) {
		IsEnabled = false;
		LOG_WARN("Failed to find a rect transform for a GUI panel, disabling");
	}

}

void GuiPanel::StartGUI() {
	Texture2D::Sptr tex = _texture != nullptr ? _texture : GuiBatcher::GetDefaultTexture();

	GuiBatcher::PushRect(glm::vec2(0, 0), _transform->GetSize(), _color, tex, _borderRadius < 0 ? GuiBatcher::GetDefaultBorderRadius() : _borderRadius);

}

void GuiPanel::FinishGUI() {
}

void GuiPanel::RenderImGui()
{
	LABEL_LEFT(ImGui::ColorEdit4, "Color ", &_color.x);
	LABEL_LEFT(ImGui::DragInt, "Radius", &_borderRadius, 1, 0, 128);
}

void GuiPanel::Update(float deltaTime) {

	//Get Window Size
	Application& app = Application::Get();
	glm::vec2 windowSize = app.GetWindowSize();

	//Update the GUI to dynamically move to the correct percent position of the screen. For example, an element dead center (50%, 50%) on a 1080p screen will stay centered when rescaled to 4k 
	GetGameObject()->Get<RectTransform>()->SetPosition({ windowSize.x * _percentOfScreenX, windowSize.y * _percentOfScreenY });

	//Use the proptortion relative to a 1920/1080 screen to properly scale it to their screen size
	//For some reason windowSize returns a value ~4x larger than it should be? Jank fix for now
	GetGameObject()->Get<RectTransform>()->SetSize({ (_proportionX / 4) * windowSize.x, (_proportionY / 4) * windowSize.y });

}

nlohmann::json GuiPanel::ToJson() const {
	return {
		{ "color",   _color },
		{ "border",  _borderRadius },
		{ "texture", _texture ? _texture->GetGUID().str() : "null" }
	};
}

GuiPanel::Sptr GuiPanel::FromJson(const nlohmann::json & blob) {
	GuiPanel::Sptr result = std::make_shared<GuiPanel>();

	result->_color = JsonGet(blob, "color", result->_color);
	result->_borderRadius = JsonGet(blob, "border", 0);
	result->_texture = ResourceManager::Get<Texture2D>(Guid(JsonGet<std::string>(blob, "texture", "null")));

	return result;
}