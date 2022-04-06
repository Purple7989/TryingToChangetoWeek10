#pragma once
#include "Application/Layers/PostProcessingLayer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture3D.h"


class ColorCorrectionEffect : public PostProcessingLayer::Effect {
public:
	MAKE_PTRS(ColorCorrectionEffect);
	Texture3D::Sptr Lut;
	Texture3D::Sptr Lut2;
	Texture3D::Sptr Lut3;

	bool Enabled = true;
	bool checked1;
	bool checked2;
	bool checked3;

	ColorCorrectionEffect();
	ColorCorrectionEffect(bool defaultLut);
	virtual ~ColorCorrectionEffect();

	virtual void Apply(const Framebuffer::Sptr& gBuffer) override;
	virtual void RenderImGui() override;

	// Inherited from IResource

	ColorCorrectionEffect::Sptr FromJson(const nlohmann::json& data);
	virtual nlohmann::json ToJson() const override;

protected:
	ShaderProgram::Sptr _shader;
	float _strength;
	float _strength2;
};

