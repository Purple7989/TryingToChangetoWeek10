
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "InteractableMenu.h"
#include "Gameplay/InputEngine.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Application/Application.h"

// Constructor Initializes Values for LERP and Set Position but Only SetPosition is being used atm
InteractableMenu::InteractableMenu()  : IComponent()
{ }
InteractableMenu::~InteractableMenu() = default;

//Move this from Awake if you want to dynamically add Menu Items
void InteractableMenu::Awake()
{
    //Every Child is a Menu Item IN THE ORDER THEY WERE MADE CHILDREN!
    for (auto& child : GetGameObject()->GetChildren()) {
        _MenuItems.push_back(child);
    }
}



nlohmann::json InteractableMenu::ToJson() const {
    return {
    };
}

std::vector<Gameplay::GameObject::Sptr> InteractableMenu::GetMenuItems() {
    return _MenuItems;
}

InteractableMenu::Sptr InteractableMenu::FromJson(const nlohmann::json & blob) {
    InteractableMenu::Sptr result = std::make_shared<InteractableMenu>();
    return result;
}
void InteractableMenu::RenderImGui() {

}

void InteractableMenu::Update(float deltaTime) {
    //Check if it's above a given element by grabbing element's position from the list
    //if( (mousePosition >= position - size/2) && (mousePosition <= position + size/2))
    //for (auto& element : _MenuItems) {
        //      Do the thing     
    //}
    
    //Controlled by a Switch Statement for now, will not be final Product, will be used in tandem with mouse control :)
    if (InputEngine::GetKeyState(GLFW_KEY_LEFT)==ButtonState::Pressed) {
        _selection-=1;
        if (_selection < 0)
            _selection=4;
    }
    if (InputEngine::GetKeyState(GLFW_KEY_RIGHT)==ButtonState::Pressed) {
        _selection += 1;
        if (_selection > 4)
            _selection = 0;
    }
    Application& app = Application::Get();

    //These DON'T all have to Load a scene! they could simply Enable/Disable Menu Items! 
    //this will be our approach for future implementation

    switch (_selection) {
    case 0:
        _MenuItems[0]->Get<GuiPanel>()->SetColor({ 1,1,1,1 });
        if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed)
            LOG_INFO(app.LoadScene("Level1.json"));
        app.LoadScene("Level1.json");
        break;
    case 1:
        _MenuItems[1]->Get<GuiPanel>()->SetColor({1,1,1,1});
        if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed)
            app.LoadScene("Level1.json");
        break;
    case 2:
        _MenuItems[2]->Get<GuiPanel>()->SetColor({1,1,1,1});
        if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed)
            app.LoadScene("Level1.json");
        break;
    case 3:
        _MenuItems[3]->Get<GuiPanel>()->SetColor({1,1,1,1});
        if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed)
            
            app.LoadScene("Level1.json");
        break;
    case 4:
        _MenuItems[4]->Get<GuiPanel>()->SetColor({ 1,1,1,1 });
        if (InputEngine::GetKeyState(GLFW_KEY_ENTER) == ButtonState::Pressed)
            app.Quit();
        break;
    default:
        LOG_WARN("Menu Items out of Selection Range!");
        break;
    }

    for (int i = 0; i < _MenuItems.size();i++) {
        if (i != _selection)
            _MenuItems[i]->Get<GuiPanel>()->SetColor({ 1,1,1,0.6 });
    }


}