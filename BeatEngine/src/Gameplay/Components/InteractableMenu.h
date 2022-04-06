#pragma once
#include "IComponent.h"
#include "GLFW/glfw3.h"
#include "Gameplay/GameObject.h"
#include <vector>

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class InteractableMenu : public Gameplay::IComponent {
public:
	//engine stuffs
	typedef std::shared_ptr<InteractableMenu> Sptr;
	InteractableMenu();
	virtual ~InteractableMenu();
	virtual void Awake() override;
	virtual void Update(float deltaTime) override;
	template<typename T>
	T Lerp(const T& p0, const T& p1, float t);
	virtual void RenderImGui() override;
	MAKE_TYPENAME(InteractableMenu);
	virtual nlohmann::json ToJson() const override;
	static InteractableMenu::Sptr FromJson(const nlohmann::json& blob);

	std::vector<Gameplay::GameObject::Sptr> GetMenuItems();


private:
	std::vector<Gameplay::GameObject::Sptr> _MenuItems;
	int _selection=0;
	
};