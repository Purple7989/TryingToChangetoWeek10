#include "GameOverScene.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

#include <filesystem>

// Graphics
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/ShaderProgram.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Textures/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"
#include "Graphics/Framebuffer.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Gameplay/Components/BeatTimer.h"
#include "Gameplay/Components/SeekBehaviour.h"
#include "Gameplay/Components/CharacterController.h"
#include "Gameplay/Components/LevelMover.h"
#include "Gameplay/Components/BackgroundMover.h"
#include "Gameplay/Components/EnvironmentMover.h"
#include "Gameplay/Components/ScoreComponent.h"
#include "Gameplay/Components/VinylAnim.h"
#include "Gameplay/Components/ForegroundMover.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/BuildingAnim.h"
#include "Gameplay/Components/SpawnLoop.h"
#include "Gameplay/Components/InteractableMenu.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"
#include "Application/Application.h"
#include "Gameplay/Components/ParticleSystem.h"
#include "Graphics/Textures/Texture3D.h"
#include "Graphics/Textures/Texture1D.h"

//Testing...
#include "SpawnFunctions.h"
//Animation
#include "Animation/MorphAnimationManager.h"
#include "Animation/MorphRenderComponent.h"




GameOverScene::GameOverScene() :
	ApplicationLayer()
{
	Name = "GameOver";
	Overrides = AppLayerFunctions::OnAppLoad;
}

GameOverScene::~GameOverScene() = default;

void GameOverScene::OnAppLoad(const nlohmann::json & config) {
	_CreateScene();
}



void GameOverScene::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {

		//NOTE This method of Scene loading is prone to breaking! 
		//For future me, if you ever have trouble loading this way, 
		//instead try using the following:
		//Scene::Sptr scene = Scene::FromJson( /*FilenameHere*/ );
		//app.LoadScene(scene);

		app.LoadScene("scene.json");
	}
	else {
		Scene::Sptr scene = std::make_shared<Scene>();

		ShaderProgram::Sptr reflectiveShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_environment_reflective.glsl" }
		});
		reflectiveShader->SetDebugName("Reflective");


		ShaderProgram::Sptr basicShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
		});
		basicShader->SetDebugName("Blinn-phong");


		ShaderProgram::Sptr specShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/textured_specular.glsl" }
		});
		specShader->SetDebugName("Textured-Specular");

		ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/screendoor_transparency.glsl" }
		});
		foliageShader->SetDebugName("Foliage");


		ShaderProgram::Sptr toonShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/toon_shading.glsl" }
		});
		toonShader->SetDebugName("Toon Shader");


		ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");

		ShaderProgram::Sptr tangentSpaceMapping = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});
		tangentSpaceMapping->SetDebugName("Tangent Space Mapping");

		ShaderProgram::Sptr multiTextureShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/vert_multitextured.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_multitextured.glsl" }
		});
		multiTextureShader->SetDebugName("Multitexturing");


		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");


		Texture2D::Sptr    boxTexture = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");
		Texture2D::Sptr    monkeyTex = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    leafTex = ResourceManager::CreateAsset<Texture2D>("textures/leaves.png");
		leafTex->SetMinFilter(MinFilter::Nearest);
		leafTex->SetMagFilter(MagFilter::Nearest);



		Texture1D::Sptr toonLut = ResourceManager::CreateAsset<Texture1D>("luts/toon-1D.png");
		toonLut->SetWrap(WrapMode::ClampToEdge);


		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/city/skybox.jpg");

		ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" }
		});

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap);
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));


		Texture3D::Sptr lut = ResourceManager::CreateAsset<Texture3D>("luts/cool.CUBE");
		Texture2D::Sptr TexQuitButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BQuit.png");
		Texture2D::Sptr StartTex = ResourceManager::CreateAsset<Texture2D>("textures/LStartPlatformTex.png");

		Texture2D::Sptr TexDimmedBG = ResourceManager::CreateAsset<Texture2D>("textures/GUI/DimBG.png");
		Texture2D::Sptr TexGameOverText = ResourceManager::CreateAsset<Texture2D>("textures/GUI/GameOverText.png");
		Texture2D::Sptr TexScoreBreakdown = ResourceManager::CreateAsset<Texture2D>("textures/GUI/ScoreBreakdown.png");
		Texture2D::Sptr TexContinueButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BContinue.png");

		Texture2D::Sptr TexNavigationLeftRight = ResourceManager::CreateAsset<Texture2D>("textures/GUI/NavigationLeftRight.png");
		Texture2D::Sptr TexNavigationUpDown = ResourceManager::CreateAsset<Texture2D>("textures/GUI/NavigationUpDown.png");
		Font::Sptr FontVCR = ResourceManager::CreateAsset<Font>("fonts/VCR.ttf", 16.f);
		FontVCR->Bake();

		Material::Sptr UIMat = ResourceManager::CreateAsset<Material>(basicShader);
		{
			UIMat->Name = "UIButton";
			UIMat->Set("u_Material.Diffuse", StartTex);
			UIMat->Set("u_Material.Shininess", 0.1f);
		}

		// Set up the scene's camera
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera");
		{
			camera->SetPostion(glm::vec3(-1.410, -3.500, 2.450));
			camera->LookAt(glm::vec3(0.0f));
			camera->SetRotation(glm::vec3(-90, -179, -163));

			Camera::Sptr cam = camera->Add<Camera>();
			auto rot = camera->Add<RotatingBehaviour>();
			rot->SetRotationSpeed({0,0,8});
			//cam->SetOrthoEnabled(true);
			//cam->SetOrthoVerticalScale(19.0f);
			//cam->SetFovRadians(105.f);
			//cam->SetNearPlane(0.3);

			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;
		}

		Application& app = Application::Get();
		glm::vec2 windowSize = app.GetWindowSize();


		/////////////////////////// UI //////////////////////////////

		//Game Over Block



		GameObject::Sptr background = scene->CreateGameObject("GameOver Dimmed Background");
		{//Dim BG
			RectTransform::Sptr transform = background->Add<RectTransform>();
			transform->SetPosition({ 0, 0 });
			transform->SetRotationDeg(0);
			transform->SetMin({ 0, 0 });

			GuiPanel::Sptr panel = background->Add<GuiPanel>(0.5, 0.5, 1920, 1080);
			panel->SetTexture(TexDimmedBG);
			panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			panel->SetBorderRadius(0);
			panel->IsEnabled = true;
		}

		GameObject::Sptr GameOverText = scene->CreateGameObject("GameOver Text");
		{//Game Over Text
			RectTransform::Sptr transform = GameOverText->Add<RectTransform>();
			transform->SetPosition({ 0, 0 });
			transform->SetRotationDeg(0);
			transform->SetMin({ 0, 0 });

			GuiPanel::Sptr panel = GameOverText->Add<GuiPanel>(0.5, 0.2, 809, 249);
			panel->SetTexture(TexGameOverText);
			panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			panel->SetBorderRadius(0);
			panel->IsEnabled = true;

		}

		GameObject::Sptr GameOverScore = scene->CreateGameObject("GameOver Score Breakdown");
		{//Score breakdown
			RectTransform::Sptr transform = GameOverScore->Add<RectTransform>();
			transform->SetPosition({ 0, 0 });
			transform->SetRotationDeg(0);
			transform->SetMin({ 0, 0 });

			GuiPanel::Sptr panel = GameOverScore->Add<GuiPanel>(0.4, 0.5, 504 * 0.75, 475 * 0.75);
			panel->SetTexture(TexScoreBreakdown);
			panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			panel->SetBorderRadius(0);
			panel->IsEnabled = true;

		}


		GameObject::Sptr QuitButton = scene->CreateGameObject("GameOver Quit Button");
		{//Quit
			RectTransform::Sptr transform = QuitButton->Add<RectTransform>();
			transform->SetPosition({ 0, 0 });
			transform->SetRotationDeg(0);
			transform->SetMin({ 0, 0 });

			GuiPanel::Sptr panel = QuitButton->Add<GuiPanel>(0.35, 0.8, 300, 150);
			panel->SetTexture(TexQuitButton);
			panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			panel->SetBorderRadius(0);
			panel->IsEnabled = true;

		}

		GameObject::Sptr ContinueButton = scene->CreateGameObject("GameOver Continue Button");
		{//Continue Button
			RectTransform::Sptr transform = ContinueButton->Add<RectTransform>();
			transform->SetPosition({ 0, 0 });
			transform->SetRotationDeg(0);
			transform->SetMin({ 0, 0 });

			GuiPanel::Sptr panel = ContinueButton->Add<GuiPanel>(0.65, 0.8, 300, 150);
			panel->SetTexture(TexContinueButton);
			panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			panel->SetBorderRadius(0);
			panel->IsEnabled = true;

		}

		GameObject::Sptr GameOverNavigation = scene->CreateGameObject("GameOver Navigation");
		{//Navigation Buttons
			RectTransform::Sptr transform = GameOverNavigation->Add<RectTransform>();
			transform->SetPosition({ 0, 0 });
			transform->SetRotationDeg(0);
			transform->SetMin({ 0, 0 });

			GuiPanel::Sptr panel = GameOverNavigation->Add<GuiPanel>(0.9, 0.93, 400 * 0.75, 150 * 0.75);
			panel->SetTexture(TexNavigationLeftRight);
			panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			panel->SetBorderRadius(0);
			panel->IsEnabled = true;

		}


		

		GameObject::Sptr MenuParent = scene->CreateGameObject("INTERACTABLE MENU ITEMS");
		{	
			MenuParent->AddChild(ContinueButton);
			MenuParent->AddChild(QuitButton);

			MenuParent->Add<InteractableMenu>();
		}


		



		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("scene-manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");

		// Send the scene to the application
		app.LoadScene(scene);

	}
}
