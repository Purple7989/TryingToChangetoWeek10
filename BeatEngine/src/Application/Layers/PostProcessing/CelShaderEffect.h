#pragma once
#include "Application/Layers/PostProcessingLayer.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture1D.h"

class CelShaderEffect : public PostProcessingLayer::Effect {
public:
	MAKE_PTRS(CelShaderEffect);

	CelShaderEffect();
	virtual ~CelShaderEffect();

	virtual void Apply(const Framebuffer::Sptr& gBuffer) override;
	virtual void RenderImGui() override;

	// Inherited from IResource

	CelShaderEffect::Sptr FromJson(const nlohmann::json& data);
	virtual nlohmann::json ToJson() const override;

protected:
	ShaderProgram::Sptr _shader;
	float               _Discard;
	Texture1D::Sptr		_ToonTerm;


};


