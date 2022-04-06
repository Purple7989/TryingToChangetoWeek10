#pragma once
#include "Application/ApplicationLayer.h"
#include "json.hpp"

/**
 * This example layer handles creating a default test scene, which we will use
 * as an entry point for creating a sample scene
 */
class GameOverScene final : public ApplicationLayer {
public:
	MAKE_PTRS(GameOverScene)

	GameOverScene();
	virtual ~GameOverScene();

	// Inherited from ApplicationLayer

	virtual void OnAppLoad(const nlohmann::json& config) override;

protected:
	void _CreateScene();
};