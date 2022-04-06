#pragma once
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components//RenderComponent.h"
#include "Animation/MorphRenderComponent.h"

class MorphAnimationManager : public Gameplay::IComponent {
public:

	static enum Animations {
		Run,
		Jump,
		Idle
	};

	class MorphAnim {
	public:
		int index;
		float secondsBetween;
		bool done;

		std::vector<Gameplay::MeshResource::Sptr> morphFrames;

		MorphAnim(const std::vector<Gameplay::MeshResource::Sptr>&, float);
		
		~MorphAnim() = default;

	};

	typedef std::shared_ptr<MorphAnimationManager> Sptr;
	MorphAnimationManager();
	~MorphAnimationManager();

	virtual void Update(float deltaTime) override;
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static MorphAnimationManager::Sptr FromJson(const nlohmann::json& data);

	void ManageInput();
	void InterpolateMeshes(float);
	void AddAnim(const std::vector<Gameplay::MeshResource::Sptr>&, float);
	void SetCurrentAnim(int);
	int GetCurrentAnim();
	void SetIdleIndex(int);
	void SetAnOrder(std::vector<int>);
	void SetContinuity(bool);

	bool currentlyAnimating = true;


	MAKE_TYPENAME(MorphAnimationManager);
	
protected:

	int currentAnimIndex = 0;
	int curIdleIndex = 0;
	int orderIndex = 0;
	std::vector<int>order;

	float m_timer = 0;

	bool isContinuous = true;
	bool futureContinuity = true;
	bool hasDeclaredOrder = false;
	bool toBePaused = false;
	bool morphOff = false;
	
	std::vector<MorphAnim>animations;
};



