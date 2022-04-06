#include "ColorCorrectionEffect.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

ColorCorrectionEffect::ColorCorrectionEffect() :
	ColorCorrectionEffect(true) { }

ColorCorrectionEffect::ColorCorrectionEffect(bool defaultLut) :
	PostProcessingLayer::Effect(),
	_shader(nullptr),
	_strength(1.0f),
	Lut(nullptr),
	Lut2(nullptr),
	Lut3(nullptr),
	checked1(true),
	checked2(false),
	checked3(false)
{
	Name = "Color Correction";
	_format = RenderTargetType::ColorRgb8;

	_shader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		{ ShaderPartType::Vertex, "shaders/vertex_shaders/fullscreen_quad.glsl" },
		{ ShaderPartType::Fragment, "shaders/fragment_shaders/post_effects/color_correction.glsl" }
	});

	if (defaultLut) {
		Lut = ResourceManager::CreateAsset<Texture3D>("luts/cool.cube");
		Lut2 = ResourceManager::CreateAsset<Texture3D>("luts/LessSaturationGreenRed.cube");
		Lut3 = ResourceManager::CreateAsset<Texture3D>("luts/Dawn.cube");
	}
}

ColorCorrectionEffect::~ColorCorrectionEffect() = default;

void ColorCorrectionEffect::Apply(const Framebuffer::Sptr& gBuffer)
{
	_shader->Bind();
	if (checked1 == true && checked2 == false && checked3 == false)
	{
		Lut->Bind(1);
	}
	if (checked2 == true && checked1 == false && checked3 == false)
	{
		Lut2->Bind(1);
	}
	if (checked3 == true && checked1 == false && checked2 == false)
	{
		Lut3->Bind(1);
	}
	_shader->SetUniform("u_Strength", _strength);
}



void ColorCorrectionEffect::RenderImGui()
{
	// 1st lut
	ImGui::Checkbox("Swap to Lut 1", &checked1);
	LABEL_LEFT(ImGui::LabelText, "LUT", Lut ? Lut->GetDebugName().c_str() : "none");
	//2nd lut
	ImGui::Checkbox("Swap to LUT 2", &checked2);
	LABEL_LEFT(ImGui::LabelText, "LUT2", Lut2 ? Lut2->GetDebugName().c_str() : "none");
	// 3rd lut
	ImGui::Checkbox("Swap to LUT 3", &checked3);
	LABEL_LEFT(ImGui::LabelText, "LUT3", Lut3 ? Lut2->GetDebugName().c_str() : "none");
	// strength value slider
	LABEL_LEFT(ImGui::SliderFloat, "Strength", &_strength, 0, 1);
}

ColorCorrectionEffect::Sptr ColorCorrectionEffect::FromJson(const nlohmann::json& data)
{
	ColorCorrectionEffect::Sptr result = std::make_shared<ColorCorrectionEffect>(false);
	result->Enabled = JsonGet(data, "enabled", true);
	result->_strength = JsonGet(data, "strength", result->_strength);
	result->Lut = ResourceManager::Get<Texture3D>(Guid(data["lut"].get<std::string>()));
	return result;
}

nlohmann::json ColorCorrectionEffect::ToJson() const
{
	return {
		{ "enabled", Enabled },
		{ "lut", Lut != nullptr ? Lut->GetGUID().str() : "null" }, 
		{ "strength", _strength }
	};
}
