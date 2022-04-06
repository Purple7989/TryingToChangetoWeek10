#include "PixelationEffect.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Application/Application.h"
#include "Gameplay/InputEngine.h"

PixelationEffect::PixelationEffect() :
	PixelationEffect(true) { }

PixelationEffect::PixelationEffect(bool defaultLut) :
	PostProcessingLayer::Effect(),
	_shader(nullptr),
	_strength(1.0f),
	Lut(nullptr),
	checked(false),
	Lut2(nullptr),
	_MousePos(0,0)
{
	Name = "Pixelation Effect";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/Pixelation.glsl" }
	});

	if (defaultLut) {
		Lut = ResourceManager::CreateAsset<Texture3D>("luts/cool.cube");
	}

	_MousePos = InputEngine::GetMousePos();

}

PixelationEffect::~PixelationEffect() = default;

void PixelationEffect::Apply(const Framebuffer::Sptr & gBuffer)
{
	Application& app = Application::Get();
	glm::vec2 _windowSize = app.GetWindowSize();

	_shader->Bind();
	if (checked == false)
	{
	//	Lut->Bind(1);
	}
	//_shader->SetUniform("u_Strength", _strength);
	_shader->SetUniform("u_ScreenSize", _windowSize);
	_shader->SetUniform("u_mousePosition", _MousePos);
}

void PixelationEffect::RenderImGui()
{
	ImGui::Checkbox("Swap LUT", &checked);
	LABEL_LEFT(ImGui::LabelText, "LUT", Lut ? Lut->GetDebugName().c_str() : "none");
}

PixelationEffect::Sptr PixelationEffect::FromJson(const nlohmann::json & data)
{
	PixelationEffect::Sptr result = std::make_shared<PixelationEffect>(false);
	result->Enabled = JsonGet(data, "enabled", true);
	result->_strength = JsonGet(data, "strength", result->_strength);
	result->Lut = ResourceManager::Get<Texture3D>(Guid(data["lut"].get<std::string>()));
	return result;
}

nlohmann::json PixelationEffect::ToJson() const
{
	return {
		{ "enabled", Enabled },
		{ "lut", Lut != nullptr ? Lut->GetGUID().str() : "null" },
		{ "strength", _strength }
	};
}