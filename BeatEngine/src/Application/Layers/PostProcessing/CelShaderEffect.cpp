#include "CelShaderEffect.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

CelShaderEffect::CelShaderEffect() :
	PostProcessingLayer::Effect(),
	_shader(nullptr),
	_Discard(0.1),
	_ToonTerm(nullptr)
{
	Name = "Cel Shader";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/cel_shader.glsl" }
	}); 
	_ToonTerm = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png");
	_ToonTerm->SetWrap(WrapMode::ClampToEdge);
}

CelShaderEffect::~CelShaderEffect() = default;

void CelShaderEffect::Apply(const Framebuffer::Sptr& gBuffer)
{
	_shader->Bind();
	_ToonTerm->Bind(1);
	_shader->SetUniform("u_Discard", _Discard);

	gBuffer->BindAttachment(RenderTargetAttachment::Depth, 1);

	//Can Link Buffers if we want :o
	//gBuffer->BindAttachment(RenderTargetAttachment::Color1, 2); // The normal buffer
}

void CelShaderEffect::RenderImGui()
{
	LABEL_LEFT(ImGui::SliderFloat,  "Discard Threshold", &_Discard, 0.f, 1.0f);

}

CelShaderEffect::Sptr CelShaderEffect::FromJson(const nlohmann::json& data)
{
	CelShaderEffect::Sptr result = std::make_shared<CelShaderEffect>();
	result->Enabled = JsonGet(data, "enabled", true);
	result->_Discard = JsonGet(data, "color", result->_Discard);
	return result; 
}

nlohmann::json CelShaderEffect::ToJson() const
{
	return {
		{ "enabled", Enabled },
		{ "Discard", _Discard },
	};
}
