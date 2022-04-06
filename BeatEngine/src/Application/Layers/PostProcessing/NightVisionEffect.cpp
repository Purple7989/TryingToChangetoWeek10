#include "NightVisionEffect.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

NightVisionEffect::NightVisionEffect() :
	NightVisionEffect(true) { }


NightVisionEffect::NightVisionEffect(bool defaultLut) :
	PostProcessingLayer::Effect(),
	_shader(nullptr)
{
	Name = "Night Vision";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/Night_Vision.glsl" }
	});

}

NightVisionEffect::~NightVisionEffect() = default;

void NightVisionEffect::Apply(const Framebuffer::Sptr& gBuffer)
{
	_shader->Bind();
}

void NightVisionEffect::RenderImGui()
{
}

NightVisionEffect::Sptr NightVisionEffect::FromJson(const nlohmann::json& data)
{
	NightVisionEffect::Sptr result = std::make_shared<NightVisionEffect>(false);
	result->Enabled = JsonGet(data, "enabled", true);
	return result;
}

nlohmann::json NightVisionEffect::ToJson() const
{
	return {
		{ "enabled", Enabled }
	};
}
