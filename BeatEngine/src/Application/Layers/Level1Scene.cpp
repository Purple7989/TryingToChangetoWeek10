#include "Level1Scene.h"

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/gtc/random.hpp>
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
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/BeatTimer.h"
#include "Gameplay/Components/SeekBehaviour.h"
#include "Gameplay/Components/CharacterController.h"
#include "Gameplay/Components/LevelMover.h"
#include "Gameplay/Components/BackgroundMover.h"
#include "Gameplay/Components/EnvironmentMover.h"
#include "Gameplay/Components/ScoreComponent.h"
#include "Gameplay/Components/VinylAnim.h"
#include "Gameplay/Components/ForegroundMover.h"
#include "Gameplay/Components/BuildingAnim.h"
#include "Gameplay/Components/SpawnLoop.h"
#include "Gameplay/Components/ShadowCamera.h"
#include "Gameplay/Components/Light.h"

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
#include "Animation/MorphRenderComponent.h"
#include "Animation/MorphAnimationManager.h"





Level1Scene::Level1Scene() :
	ApplicationLayer()
{
	Name = "Level1";
	Overrides = AppLayerFunctions::OnAppLoad;
}

Level1Scene::~Level1Scene() = default;

void Level1Scene::OnAppLoad(const nlohmann::json & config) {
	_CreateScene();
}



void Level1Scene::_CreateScene()
{
	using namespace Gameplay;
	using namespace Gameplay::Physics;

	Application& app = Application::Get();

	bool loadScene = false;
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene && std::filesystem::exists("scene.json")) {
		app.LoadScene("scene.json");
	}
	else {
		 Scene::Sptr scene = std::make_shared<Scene>();
		
	/*	 ShaderProgram::Sptr reflectiveShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_environment_reflective.glsl" }
		});
		reflectiveShader->SetDebugName("Reflective");
	*/
		
		 ShaderProgram::Sptr basicShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
		});
		basicShader->SetDebugName("Blinn-phong");

		//ShaderProgram::Sptr AnimatedShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
		//	{ ShaderPartType::Vertex, "shaders/geometry_shader/Animation.glsl" },
		//	{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
		//});
		//basicShader->SetDebugName("Animated Object Material");


		ShaderProgram::Sptr deferredForward = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
	/*
		 ShaderProgram::Sptr specShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/textured_specular.glsl" }
		});
		specShader->SetDebugName("Textured-Specular");
	*/	

		 ShaderProgram::Sptr foliageShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		foliageShader->SetDebugName("Foliage");
		
	/*
		 ShaderProgram::Sptr toonShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/toon_shading.glsl" }
		});
		toonShader->SetDebugName("Toon Shader");
	*/
	// This shader handles our cel shading example
		ShaderProgram::Sptr celShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/cel_shader.glsl" }
		});
		celShader->SetDebugName("Cel Shader");
		
		 ShaderProgram::Sptr displacementShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/displacement_mapping.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/deferred_forward.glsl" }
		});
		displacementShader->SetDebugName("Displacement Mapping");
	/*
		 ShaderProgram::Sptr tangentSpaceMapping = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_tangentspace_normal_maps.glsl" }
		});
		tangentSpaceMapping->SetDebugName("Tangent Space Mapping");
	*/
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
		
		
		 TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/skybox/skybox.png");
		
		 ShaderProgram::Sptr      skyboxShader = ResourceManager::CreateAsset<ShaderProgram>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" }
		});

#pragma region Basic Texture Creation
		 Texture2DDescription singlePixelDescriptor;
		 singlePixelDescriptor.Width = singlePixelDescriptor.Height = 1;
		 singlePixelDescriptor.Format = InternalFormat::RGB8;

		 float normalMapDefaultData[3] = { 0.5f, 0.5f, 1.0f };
		 Texture2D::Sptr normalMapDefault = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		 normalMapDefault->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, normalMapDefaultData);

		 float solidBlack[3] = { 0.5f, 0.5f, 0.5f };
		 Texture2D::Sptr solidBlackTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		 solidBlackTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidBlack);

		 float solidGrey[3] = { 0.0f, 0.0f, 0.0f };
		 Texture2D::Sptr solidGreyTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		 solidGreyTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidGrey);

		 float solidWhite[3] = { 1.0f, 1.0f, 1.0f };
		 Texture2D::Sptr solidWhiteTex = ResourceManager::CreateAsset<Texture2D>(singlePixelDescriptor);
		 solidWhiteTex->LoadData(1, 1, PixelFormat::RGB, PixelType::Float, solidWhite);

#pragma endregion 

		 // Setting up our enviroment map
		 scene->SetSkyboxTexture(testCubemap);
		 scene->SetSkyboxShader(skyboxShader);
		 // Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up 
		 scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		 // Loading in a color lookup table
		 // Warm
		 Texture3D::Sptr lutWarm = ResourceManager::CreateAsset<Texture3D>("luts/Dawn.CUBE");
		 // Cool
		 Texture3D::Sptr lutCool = ResourceManager::CreateAsset<Texture3D>("luts/NightCity.CUBE");
		 // Custom
		 Texture3D::Sptr lutCustom = ResourceManager::CreateAsset<Texture3D>("luts/shrooms.CUBE");

		 // Configure the color correction LUT
		 scene->SetColorLUT(lutCustom);
		 //scene->SetColorLUTCool(lutWarm);
		// scene->SetColorLUTCustom(lutCustom);
		 
		 MeshResource::Sptr SmallPlatform = ResourceManager::CreateAsset<MeshResource>("HaloBasicPlatform.obj");
		 MeshResource::Sptr WallJumpBuilding = ResourceManager::CreateAsset<MeshResource>("walljumpbuilding.obj");
		 MeshResource::Sptr WallJumpSign = ResourceManager::CreateAsset<MeshResource>("walljumpsign.obj");
		 MeshResource::Sptr WallJump = ResourceManager::CreateAsset<MeshResource>("WallJumpV6.obj");
		 MeshResource::Sptr BeatGem = ResourceManager::CreateAsset<MeshResource>("Gem.obj");
		 MeshResource::Sptr Vinyl = ResourceManager::CreateAsset<MeshResource>("VinylV2.obj");
		 MeshResource::Sptr CD = ResourceManager::CreateAsset<MeshResource>("CDwithUnwrap.obj");
		 MeshResource::Sptr Building = ResourceManager::CreateAsset<MeshResource>("RBuilding.obj");
		 MeshResource::Sptr KBuilding1Mesh = ResourceManager::CreateAsset<MeshResource>("KBuilding.obj");
		 MeshResource::Sptr KBuilding2Mesh = ResourceManager::CreateAsset<MeshResource>("KBuilding2.obj");
		 MeshResource::Sptr KBuilding3Mesh = ResourceManager::CreateAsset<MeshResource>("KBuilding3.obj");
		 MeshResource::Sptr OvalBuilding = ResourceManager::CreateAsset<MeshResource>("OvalBuilding.obj");
		 MeshResource::Sptr CharacterMesh = ResourceManager::CreateAsset<MeshResource>("discobot.obj");
		 MeshResource::Sptr DiscoBallMesh = ResourceManager::CreateAsset<MeshResource>("DiscoBall2.obj");
		 MeshResource::Sptr StartPlatform = ResourceManager::CreateAsset<MeshResource>("startbuilding.obj");
		 MeshResource::Sptr Car1Mesh = ResourceManager::CreateAsset<MeshResource>("FutureCar1.obj");
		 MeshResource::Sptr SemiTruckMesh = ResourceManager::CreateAsset<MeshResource>("Semitruck.obj");
		 MeshResource::Sptr PickupTruckMesh = ResourceManager::CreateAsset<MeshResource>("FuturePickup.obj");
		 MeshResource::Sptr SmallWallJump = ResourceManager::CreateAsset<MeshResource>("SmallWallJump.obj");
		 MeshResource::Sptr SuperSmallWallJump = ResourceManager::CreateAsset<MeshResource>("SuperSmallWallJump.obj");
		 MeshResource::Sptr FallingPlat = ResourceManager::CreateAsset<MeshResource>("pianoplatform.obj");
		 MeshResource::Sptr HalfCirclePlat = ResourceManager::CreateAsset<MeshResource>("HalfCriclePlat.obj");
		 MeshResource::Sptr StairsRight = ResourceManager::CreateAsset<MeshResource>("SlantedStairCase.obj");
		 MeshResource::Sptr StairsLeft = ResourceManager::CreateAsset<MeshResource>("StairCaseL.obj");
		 MeshResource::Sptr Speaker = ResourceManager::CreateAsset<MeshResource>("speaker.obj");
		 MeshResource::Sptr SquarePlat = ResourceManager::CreateAsset<MeshResource>("SquarePlatform.obj");
		 MeshResource::Sptr BeatBarVinylMesh = ResourceManager::CreateAsset<MeshResource>("beatbar.obj");
		 MeshResource::Sptr NeedleMesh = ResourceManager::CreateAsset<MeshResource>("needle.obj");
		 MeshResource::Sptr FloatingLight = ResourceManager::CreateAsset<MeshResource>("FloatingStreetLight.obj");
		 MeshResource::Sptr DiscoBotMesh1 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run1.obj");
		 MeshResource::Sptr DiscoBotMesh2 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run2.obj");
		 MeshResource::Sptr DiscoBotMesh3 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run3.obj");
		 MeshResource::Sptr DiscoBotMesh4 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run4.obj");
		 MeshResource::Sptr DiscoBotMesh5 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run5.obj");
		 MeshResource::Sptr DiscoBotMesh6 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run6.obj");
		 MeshResource::Sptr DiscoBotMesh7 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run7.obj");
		 MeshResource::Sptr DiscoBotMesh8 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run8.obj");
		 MeshResource::Sptr DiscoBotMesh9 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/run9.obj");
		 MeshResource::Sptr BotJump1 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump1.obj");
		 MeshResource::Sptr BotJump2 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump2.obj");
		 MeshResource::Sptr BotJump3 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump3.obj");
		 MeshResource::Sptr BotJump4 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump4.obj");
		 MeshResource::Sptr BotJump5 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump5.obj");
		 MeshResource::Sptr BotJump6 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump6.obj");
		 MeshResource::Sptr BotJump7 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/jump7.obj");
		 MeshResource::Sptr Idle1 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/idle1.obj");
		 MeshResource::Sptr Idle2 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/idle2.obj");
		 MeshResource::Sptr Idle3 = ResourceManager::CreateAsset<MeshResource>("CharacterAnims/idle3.obj");

		 Texture2D::Sptr StartTex = ResourceManager::CreateAsset<Texture2D>("textures/startbuildingtex.png");
		 Texture2D::Sptr SmallTex = ResourceManager::CreateAsset<Texture2D>("textures/HaloBasicPlatformTex.png");
		 Texture2D::Sptr WallJumpBuildingTex = ResourceManager::CreateAsset<Texture2D>("textures/walljumpbuildingTEX.png");
		 Texture2D::Sptr WallJumpSignTex = ResourceManager::CreateAsset<Texture2D>("textures/walljumpsignTEX.png");
		 Texture2D::Sptr VinylTex = ResourceManager::CreateAsset<Texture2D>("textures/VinylTex.png");
		 Texture2D::Sptr CDTex = ResourceManager::CreateAsset<Texture2D>("textures/CDTex.png");
		 Texture2D::Sptr GemTex = ResourceManager::CreateAsset<Texture2D>("textures/Gem.png");
		 Texture2D::Sptr GemOff = ResourceManager::CreateAsset<Texture2D>("textures/GemOff.png");
		 Texture2D::Sptr GemAnticipation = ResourceManager::CreateAsset<Texture2D>("textures/GemYellow.png");
		 Texture2D::Sptr CharacterTex = ResourceManager::CreateAsset<Texture2D>("textures/shirt.png");
		 Texture2D::Sptr LoseScreenTex = ResourceManager::CreateAsset<Texture2D>("textures/Game_Over_Screen.png");
		 Texture2D::Sptr SmallWallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/SmallWallJumpTexBlue.png");
		 Texture2D::Sptr SuperSmallWallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/SmallWallJumpTexRed.png");
		 Texture2D::Sptr WallJumpTex = ResourceManager::CreateAsset<Texture2D>("textures/WallJumpTex.png");
		 Texture2D::Sptr Car1Tex = ResourceManager::CreateAsset<Texture2D>("textures/FutureCarReTex2.png");
		 Texture2D::Sptr SemiTruckTex = ResourceManager::CreateAsset<Texture2D>("textures/SemiTruckTexV2.png");
		 Texture2D::Sptr PickupTruckTex = ResourceManager::CreateAsset<Texture2D>("textures/PickupTruckTexV3.png");
		 Texture2D::Sptr KBuilding1Tex = ResourceManager::CreateAsset<Texture2D>("textures/KBuildingTex.png");
		 Texture2D::Sptr KBuilding2Tex = ResourceManager::CreateAsset<Texture2D>("textures/KBuilding2Tex.png");
		 Texture2D::Sptr KBuilding3Tex = ResourceManager::CreateAsset<Texture2D>("textures/KBuilding3Tex.png");
		 Texture2D::Sptr BuildingTex = ResourceManager::CreateAsset<Texture2D>("textures/RBuildingTex.png");
		 Texture2D::Sptr OvalBuildingTex = ResourceManager::CreateAsset<Texture2D>("textures/OvalBuildingTex.png");
		 Texture2D::Sptr DiscoBallTex = ResourceManager::CreateAsset<Texture2D>("textures/DiscoBallTexV2.png");
		 Texture2D::Sptr FallingPlatTex = ResourceManager::CreateAsset<Texture2D>("textures/pianotex.png");
		 Texture2D::Sptr HalfCirclePlatTex = ResourceManager::CreateAsset<Texture2D>("textures/halfCircleTex.png");
		 Texture2D::Sptr TexBeatLogo = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BeatLogo.png");
		 Texture2D::Sptr TexPlayButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BPlay.png");
		 Texture2D::Sptr TexOptionsButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BOptions.png");
		 Texture2D::Sptr TexMusicButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BMusic.png");
		 Texture2D::Sptr TexCreditsButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BCredits.png");
		 Texture2D::Sptr TexQuitButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BQuit.png");
		 Texture2D::Sptr TexResumeButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BResume.png");
		 Texture2D::Sptr TexResyncButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BResync.png");
		 Texture2D::Sptr TexContinueButton = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BContinue.png");
		 Texture2D::Sptr TexPauseMenu = ResourceManager::CreateAsset<Texture2D>("textures/GUI/PauseMenuBG.png");
		 Texture2D::Sptr TexDimmedBG = ResourceManager::CreateAsset<Texture2D>("textures/GUI/DimBG.png");
		 Texture2D::Sptr TexScoreBreakdown = ResourceManager::CreateAsset<Texture2D>("textures/GUI/ScoreBreakdown.png");
		 Texture2D::Sptr TexGameOverText = ResourceManager::CreateAsset<Texture2D>("textures/GUI/GameOverText.png");
		 Texture2D::Sptr TexMovementTutorial = ResourceManager::CreateAsset<Texture2D>("textures/GUI/Movement.png");
		 Texture2D::Sptr TexWallJumpTutorial = ResourceManager::CreateAsset<Texture2D>("textures/GUI/WallJump.png");
		 Texture2D::Sptr TexBeatGemTutorial = ResourceManager::CreateAsset<Texture2D>("textures/GUI/BeatGems.png");
		 Texture2D::Sptr TexVinylsTutorial = ResourceManager::CreateAsset<Texture2D>("textures/GUI/Vinyls.png");
		 Texture2D::Sptr StairsRightTex = ResourceManager::CreateAsset<Texture2D>("textures/StairCaseTex.png");
		 Texture2D::Sptr StairsLeftTex = ResourceManager::CreateAsset<Texture2D>("textures/StarTexL.png");
		 Texture2D::Sptr SpeakerTex = ResourceManager::CreateAsset<Texture2D>("textures/speakertex.png");
		 Texture2D::Sptr SquarePlatTex = ResourceManager::CreateAsset<Texture2D>("textures/SquarePlatformTex.png");
		 Texture2D::Sptr FloatingLightTex = ResourceManager::CreateAsset<Texture2D>("textures/StreetLightTex.png");
		 Texture2D::Sptr TexVinylBeatBar = ResourceManager::CreateAsset<Texture2D>("textures/GUI/Vinyl_Beat_Bar.png");
		 Texture2D::Sptr TexBeatBarNeedle = ResourceManager::CreateAsset<Texture2D>("textures/GUI/needletexture.png");
		 Texture2D::Sptr TexScoreDisplay = ResourceManager::CreateAsset<Texture2D>("textures/GUI/ScoreDisplay.png");
		 Texture2D::Sptr TexNavigationLeftRight = ResourceManager::CreateAsset<Texture2D>("textures/GUI/NavigationLeftRight.png");
		 Texture2D::Sptr TexNavigationUpDown = ResourceManager::CreateAsset<Texture2D>("textures/GUI/NavigationUpDown.png");
		
		 Font::Sptr FontVCR = ResourceManager::CreateAsset<Font>("fonts/VCR.ttf", 16.f);
		 FontVCR->Bake();

		 // In Order For the Toon Shader to Work you must include this line on each object using the shader
		 // boxMaterial->Set("s_ToonTerm", toonLut);
		 // boxMaterial->Set("u_Material.Steps", 8);
		
	/*	 Material::Sptr StartPlatformMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			StartPlatformMaterial->Name = "StartPlatform";
			StartPlatformMaterial->Set("u_Material.Diffuse", StartTex);
			StartPlatformMaterial->Set("u_Material.Shininess", 0.1f);
			StartPlatformMaterial->Set("s_ToonTerm", toonLut);
			StartPlatformMaterial->Set("u_Material.Steps", 8);
		}*/

		 Material::Sptr StartPlatformMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		 {
			 StartPlatformMaterial->Name = "StartPlatform";
			 StartPlatformMaterial->Set("u_Material.AlbedoMap", StartTex);
			 StartPlatformMaterial->Set("u_Material.NormalMap", normalMapDefault);
			 StartPlatformMaterial->Set("u_Material.Shininess", 0.1f);
		 }

		 Material::Sptr WallJumpBuildingMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		 {
			 WallJumpBuildingMaterial->Name = "WallJumpBuilding";
			 WallJumpBuildingMaterial->Set("u_Material.AlbedoMap", WallJumpBuildingTex);
			 WallJumpBuildingMaterial->Set("u_Material.NormalMap", normalMapDefault);
			 WallJumpBuildingMaterial->Set("u_Material.Shininess", 0.1f);
		 }

		 Material::Sptr WallJumpSignMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		 {
			 WallJumpSignMaterial->Name = "WallJumpSign";
			 WallJumpSignMaterial->Set("u_Material.AlbedoMap", WallJumpSignTex);
			 WallJumpSignMaterial->Set("u_Material.NormalMap", normalMapDefault);
			 WallJumpSignMaterial->Set("u_Material.Shininess", 0.1f);
		 }
		
		 Material::Sptr UIMat = ResourceManager::CreateAsset<Material>(basicShader);
		{
			UIMat->Name = "UIButton";
			UIMat->Set("u_Material.Diffuse", StartTex);
			UIMat->Set("u_Material.Shininess", 0.1f);
		}
		
		Material::Sptr SmallPlatformMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SmallPlatformMaterial->Name = "SmallPlatform";
			SmallPlatformMaterial->Set("u_Material.AlbedoMap", SmallTex);
			SmallPlatformMaterial->Set("u_Material.NormalMap", normalMapDefault);
			SmallPlatformMaterial->Set("u_Material.Shininess", 0.1f);
		}
		
		Material::Sptr WallJumpMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			WallJumpMaterial->Name = "WallJump";
			WallJumpMaterial->Set("u_Material.AlbedoMap", WallJumpTex);
			WallJumpMaterial->Set("u_Material.NormalMap", normalMapDefault);
			WallJumpMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr BeatGemMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			BeatGemMaterial->Name = "BeatGem";
			BeatGemMaterial->Set("u_Material.AlbedoMap", GemTex);
			BeatGemMaterial->Set("u_Material.NormalMap", normalMapDefault);
			BeatGemMaterial->Set("u_Material.Shininess", 0.1f);
		}
		//	GemAnticipation	 
		Material::Sptr BeatGemAnticipationMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			BeatGemAnticipationMaterial->Name = "BeatGemAnticipationMaterial";
			BeatGemAnticipationMaterial->Set("u_Material.AlbedoMap", GemAnticipation);
			BeatGemAnticipationMaterial->Set("u_Material.NormalMap", normalMapDefault);
			BeatGemAnticipationMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr BeatGemOffMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			BeatGemOffMaterial->Name = "BeatGemOff";
			BeatGemOffMaterial->Set("u_Material.AlbedoMap", GemOff);
			BeatGemOffMaterial->Set("u_Material.NormalMap", normalMapDefault);
			BeatGemOffMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr VinylMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			VinylMaterial->Name = "Vinyl";
			VinylMaterial->Set("u_Material.AlbedoMap", VinylTex);
			VinylMaterial->Set("u_Material.NormalMap", normalMapDefault);
			VinylMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr CDMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			CDMaterial->Name = "CD";
			CDMaterial->Set("u_Material.AlbedoMap", CDTex);
			CDMaterial->Set("u_Material.NormalMap", normalMapDefault);
			CDMaterial->Set("u_Material.Shininess", 0.1f);
		}
		
		Material::Sptr CharacterMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			CharacterMaterial->Name = "Character";
			CharacterMaterial->Set("u_Material.AlbedoMap", CharacterTex);
			CharacterMaterial->Set("u_Material.NormalMap", normalMapDefault);
			CharacterMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr DiscoBallMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			DiscoBallMaterial->Name = "DiscoBall";
			DiscoBallMaterial->Set("u_Material.AlbedoMap", DiscoBallTex);
			DiscoBallMaterial->Set("u_Material.NormalMap", normalMapDefault);
			DiscoBallMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr LoseScreenMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			LoseScreenMaterial->Name = "Lose Screen";
			LoseScreenMaterial->Set("u_Material.AlbedoMap", LoseScreenTex);
			LoseScreenMaterial->Set("u_Material.NormalMap", normalMapDefault);
			LoseScreenMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr Car1Material = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			Car1Material->Name = "Car1";
			Car1Material->Set("u_Material.AlbedoMap", Car1Tex);
			Car1Material->Set("u_Material.NormalMap", normalMapDefault);
			Car1Material->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SemiTruckMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SemiTruckMaterial->Name = "Semi1";
			SemiTruckMaterial->Set("u_Material.AlbedoMap", SemiTruckTex);
			SemiTruckMaterial->Set("u_Material.NormalMap", normalMapDefault);
			SemiTruckMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr PickupTruckMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			PickupTruckMaterial->Name = "Pickup1";
			PickupTruckMaterial->Set("u_Material.AlbedoMap", PickupTruckTex);
			PickupTruckMaterial->Set("u_Material.NormalMap", normalMapDefault);
			PickupTruckMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr BuildingMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			BuildingMaterial->Name = "Building";
			BuildingMaterial->Set("u_Material.AlbedoMap", BuildingTex);
			BuildingMaterial->Set("u_Material.NormalMap", normalMapDefault);
			BuildingMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr KBuildingMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			KBuildingMaterial->Name = "KBuilding";
			KBuildingMaterial->Set("u_Material.AlbedoMap", KBuilding1Tex);
			KBuildingMaterial->Set("u_Material.NormalMap", normalMapDefault);
			KBuildingMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr KBuilding2Material = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			KBuilding2Material->Name = "KBuilding2";
			KBuilding2Material->Set("u_Material.AlbedoMap", KBuilding2Tex);
			KBuilding2Material->Set("u_Material.NormalMap", normalMapDefault);
			KBuilding2Material->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr KBuilding3Material = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			KBuilding3Material->Name = "KBuilding3";
			KBuilding3Material->Set("u_Material.AlbedoMap", KBuilding3Tex);
			KBuilding3Material->Set("u_Material.NormalMap", normalMapDefault);
			KBuilding3Material->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SmallWallJumpMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SmallWallJumpMaterial->Name = "Small Wall Jump";
			SmallWallJumpMaterial->Set("u_Material.AlbedoMap", SmallWallJumpTex);
			SmallWallJumpMaterial->Set("u_Material.NormalMap", normalMapDefault);
			SmallWallJumpMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SuperSmallWallJumpMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SuperSmallWallJumpMaterial->Name = "Super Small Wall Jump";
			SuperSmallWallJumpMaterial->Set("u_Material.AlbedoMap", SuperSmallWallJumpTex);
			SuperSmallWallJumpMaterial->Set("u_Material.NormalMap", normalMapDefault);
			SuperSmallWallJumpMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr PianoMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			PianoMaterial->Name = "Piano";
			PianoMaterial->Set("u_Material.AlbedoMap", FallingPlatTex);
			PianoMaterial->Set("u_Material.NormalMap", normalMapDefault);
			PianoMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr HalfCirclePlatMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			HalfCirclePlatMaterial->Name = "Half Circle Plat";
			HalfCirclePlatMaterial->Set("u_Material.AlbedoMap", HalfCirclePlatTex);
			HalfCirclePlatMaterial->Set("u_Material.NormalMap", normalMapDefault);
			HalfCirclePlatMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr StairsRightMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			StairsRightMaterial->Name = "Stairs Right";
			StairsRightMaterial->Set("u_Material.AlbedoMap", StairsRightTex);
			StairsRightMaterial->Set("u_Material.NormalMap", normalMapDefault);
			StairsRightMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr StairsLeftMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			StairsLeftMaterial->Name = "Stairs Left";
			StairsLeftMaterial->Set("u_Material.AlbedoMap", StairsLeftTex);
			StairsLeftMaterial->Set("u_Material.NormalMap", normalMapDefault);
			StairsLeftMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SpeakerMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SpeakerMaterial->Name = "Speaker Material";
			SpeakerMaterial->Set("u_Material.AlbedoMap", SpeakerTex);
			SpeakerMaterial->Set("u_Material.NormalMap", normalMapDefault);
			SpeakerMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr SquarePlatMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			SquarePlatMaterial->Name = "Square Platform";
			SquarePlatMaterial->Set("u_Material.AlbedoMap", SquarePlatTex);
			SquarePlatMaterial->Set("u_Material.NormalMap", normalMapDefault);
			SquarePlatMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr FloatingLightMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			FloatingLightMaterial->Name = "FLoating Light";
			FloatingLightMaterial->Set("u_Material.AlbedoMap", FloatingLightTex);
			FloatingLightMaterial->Set("u_Material.NormalMap", normalMapDefault);
			FloatingLightMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr OvalBuildingMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			OvalBuildingMaterial->Name = "Oval Building";
			OvalBuildingMaterial->Set("u_Material.AlbedoMap", OvalBuildingTex);
			OvalBuildingMaterial->Set("u_Material.NormalMap", normalMapDefault);
			OvalBuildingMaterial->Set("u_Material.Shininess", 0.1f);
		}
		Material::Sptr BeatBarVinylMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			BeatBarVinylMaterial->Name = "Beat Bar Vinyl";
			BeatBarVinylMaterial->Set("u_Material.AlbedoMap", TexVinylBeatBar);
			BeatBarVinylMaterial->Set("u_Material.NormalMap", normalMapDefault);
			BeatBarVinylMaterial->Set("u_Material.Shininess", 0.1f);
			BeatBarVinylMaterial->Set("u_Material.IsLit", true);

		}

		Material::Sptr BeatBarNeedleMaterial = ResourceManager::CreateAsset<Material>(deferredForward);
		{
			BeatBarNeedleMaterial->Name = "Beat Bar Needle";
			BeatBarNeedleMaterial->Set("u_Material.AlbedoMap", TexBeatBarNeedle);
			BeatBarNeedleMaterial->Set("u_Material.NormalMap", normalMapDefault);
			BeatBarNeedleMaterial->Set("u_Material.Shininess", 0.1f);
		}

		// Set up the scene's camera
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera");
		{
			camera->SetPostion(glm::vec3(-1.880, -5.320, 0.700));
			camera->LookAt(glm::vec3(0.0f));
			camera->SetRotation(glm::vec3(90, 0, 0));

			Camera::Sptr cam = camera->Add<Camera>();
			//cam->SetOrthoEnabled(true);
			//cam->SetOrthoVerticalScale(19.0f);
			cam->SetFovRadians(1.39626f);
			//cam->SetNearPlane(0.3);

			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;
		}

		//This is a game object built purely to manage game systems i.e. Scene Swaps
		//Level Spawning, Score Counting, and a few miscellaneous systems
		GameObject::Sptr GameManager = scene->CreateGameObject("GameManager");
		{
			//Pos-Rot-Scale Doesn't matter
			//RigidBody::Sptr physics = GameManager->Add<RigidBody>(RigidBodyType::Kinematic);
			GameManager->Add<BeatTimer>();
			GameManager->Add<SpawnLoop>();
			//ScoreComponent
			//LevelSpawningComponent
			//Scene Swapper
		}

		//float const distanceFromBlock = 20.f;

		// Background and forground vehicles\\
		Give these Parents for Foreground/Background Blocks if we have enough objects to do that with!
		SpawnFunctions::SpawnBackGroundCar(scene, Car1Mesh, Car1Material, "Car1", glm::vec3(14.870f, 7.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnFunctions::SpawnBackGroundCar(scene, SemiTruckMesh, SemiTruckMaterial, "Semi1", glm::vec3(28.870f, 7.80f, 2.7f), glm::vec3(90.0f, 0.0f, -90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnFunctions::SpawnForeGroundCar(scene, Car1Mesh, Car1Material, "Car2", glm::vec3(-9.970f, 0.470f, -2.990f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnFunctions::SpawnForeGroundCar(scene, PickupTruckMesh, PickupTruckMaterial, "Pickup1", glm::vec3(-18.970f, 0.470f, -1.90f), glm::vec3(90.0f, 0.0f, 90.0f), glm::vec3(0.250f, 0.250f, 0.250f));
		SpawnFunctions::SpawnBackGroundBuilding(scene, KBuilding1Mesh, KBuildingMaterial, "KBuilding1", glm::vec3(-1.0f, 21.880f, -46.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.780f, 1.470f, 1.0f));
		SpawnFunctions::SpawnBackGroundBuilding(scene, KBuilding2Mesh, KBuilding2Material, "KBuilding2", glm::vec3(25.670, 21.880f, -46.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.780f, 1.470f, 1.0f));
		SpawnFunctions::SpawnBackGroundBuilding(scene, KBuilding3Mesh, KBuilding3Material, "KBuilding3", glm::vec3(-30.530f, 21.880f, -46.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.780f, 1.470f, 1.0f));
		SpawnFunctions::SpawnBackGroundBuilding(scene, OvalBuilding, OvalBuildingMaterial, "OvalBuilding", glm::vec3(13.730f, 27.510f, -46.040f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.720f, 3.500f, 1.0f));
		
		// Tutorial
		SpawnFunctions::SpawnStartPlat(scene, StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-11.640f, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-8.620f, 5.610f, -2.200f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-5.290f, 5.610f, -4.040f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(1.460f, 5.610f, 4.400f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(1.320f, 5.610f, -5.530f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(4.680f, 5.610f, -4.590f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnWallJumpBuilding(scene, WallJumpBuilding, WallJumpBuildingMaterial, "Wall Jump", glm::vec3(-0.590f, 5.610f, -0.660f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.070f, 0.090f, 0.090f));
		SpawnFunctions::SpawnWallJumpSign(scene, WallJumpSign, WallJumpSignMaterial, "Wall Jump", glm::vec3(-0.800f, 5.610f, -0.170f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.070f, 0.090f, 0.090f));
		SpawnFunctions::SpawnGem(scene, BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem", 4, glm::vec3(-2.410f, 5.610f, -5.460f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnFunctions::SpawnCollectable(scene, Vinyl, VinylMaterial, "Vinyl", glm::vec3(1.430f, 5.610f, 4.960f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnFunctions::SpawnStartPlat(scene, StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		float adjustment = 30;
		
		// 1st Block		
		SpawnFunctions::SpawnStartPlat(scene, StartPlatform, StartPlatformMaterial, "StartPlatform", glm::vec3(-15.820f + adjustment, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-10.460f + adjustment, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(-3.940f + adjustment, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(2.200f + adjustment, 5.610f, -1.770f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnObj(scene, SmallPlatform, SmallPlatformMaterial, "Small Platform", glm::vec3(6.500f + adjustment, 5.610f, -4.150f), glm::vec3(180.0f, 0.0f, 180.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		SpawnFunctions::SpawnGem(scene, BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem", 4, glm::vec3(-0.520f + adjustment, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnFunctions::SpawnGem(scene, BeatGem, BeatGemMaterial, BeatGemOffMaterial, "BeatGem", 4, glm::vec3(-6.940f + adjustment, 5.610f, -3.160f), glm::vec3(90.0f, 0.0f, 180.0f), glm::vec3(0.500f, 0.500f, 0.500f));
		SpawnFunctions::SpawnCollectable(scene, Vinyl, VinylMaterial, "Vinyl", glm::vec3(5.210f + adjustment, 5.610f, 0.080f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnFunctions::SpawnStartPlat(scene, StartPlatform, StartPlatformMaterial, "EndPlatform", glm::vec3(9.180f + adjustment, 5.610f, -9.10f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(0.350f, 0.350f, 0.350f));
		
		// CDs for Block 1
		SpawnFunctions::SpawnCD(scene, CD, CDMaterial, "CD", glm::vec3(-10.220f + adjustment, 5.610f, -3.220f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnFunctions::SpawnCD(scene, CD, CDMaterial, "CD", glm::vec3(-6.970f + adjustment, 5.610f, -2.810f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnFunctions::SpawnCD(scene, CD, CDMaterial, "CD", glm::vec3(-3.770f + adjustment, 5.610f, -3.190f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnFunctions::SpawnCD(scene, CD, CDMaterial, "CD", glm::vec3(-0.620f + adjustment, 5.610f, -2.380f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));
		SpawnFunctions::SpawnCD(scene, CD, CDMaterial, "CD", glm::vec3(2.140f + adjustment, 5.610f, -0.770f), glm::vec3(90.000f, 0.0f, 90.000f), glm::vec3(1.000f, 1.000f, 1.000f));

		Application& app = Application::Get();
		glm::vec2 windowSize = app.GetWindowSize();

		//trust me if I could find a better way i'd have done it but rn we can't pass materials to 
		//the Material Swap Component so we instead add Dummy Children Objects to hold the Materials instead.
		{
			GameObject::Sptr MaterialDummyOn = scene->CreateGameObject("Material Dummy On");
			RenderComponent::Sptr renderer = MaterialDummyOn->Add<RenderComponent>();
			renderer->SetMesh(BeatGem);
			renderer->SetMaterial(BeatGemMaterial);
			renderer->IsEnabled = false;
		}
		{
			GameObject::Sptr MaterialDummyOff = scene->CreateGameObject("Material Dummy Off");
			RenderComponent::Sptr renderer = MaterialDummyOff->Add<RenderComponent>();
			renderer->SetMesh(BeatGem);
			renderer->SetMaterial(BeatGemOffMaterial);
			renderer->IsEnabled = false;
		}
		{
			GameObject::Sptr MaterialDummyAnticipation = scene->CreateGameObject("MaterialDummyAnticipation");
			RenderComponent::Sptr renderer = MaterialDummyAnticipation->Add<RenderComponent>();
			renderer->SetMesh(BeatGem);
			renderer->SetMaterial(BeatGemAnticipationMaterial);
			renderer->IsEnabled = false;
		}

		// Player:
		GameObject::Sptr character = scene->CreateGameObject("Character/Player");
		{
			// Set position in the scene
			character->SetPostion(glm::vec3(-16.820f, 5.710f, -3.000f));
			character->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			character->SetScale(glm::vec3(0.7f, 0.7f, 0.7f));

			// Add some behaviour that relies on the physics body
			//character->Add<JumpBehaviour>();
			//character->Add<JumpBehaviour>();
			character->Add<CharacterController>();

			// Create and attach a renderer for the paddle
			RenderComponent::Sptr renderer = character->Add<RenderComponent>();
			renderer->SetMesh(CharacterMesh);
			renderer->SetMaterial(CharacterMaterial);

			// Add a kinematic rigid body to the paddle
			RigidBody::Sptr physics = character->Add<RigidBody>(RigidBodyType::Dynamic);
			auto rb = physics->AddCollider(BoxCollider::Create(glm::vec3(0.2, 0.85, 0.15)));
			rb->SetPosition(glm::vec3(0.0, 0.75, 0.0));

			TriggerVolume::Sptr volume = character->Add<TriggerVolume>();
			volume->SetFlags(TriggerTypeFlags::Statics | TriggerTypeFlags::Kinematics);

			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(0.3f, 0.3f, 0.5f));
			collider->SetPosition(glm::vec3(0.f, 0.25f, 0.f));

			BoxCollider::Sptr collider2 = BoxCollider::Create(glm::vec3(0.3f, 0.3f, 0.3f));
			collider2->SetPosition(glm::vec3(3.f, 0.25f, 0.f));

			volume->AddCollider(collider);
			volume->AddCollider(collider2);

			MorphRenderComponent::Sptr morph1 = character->Add<MorphRenderComponent>(CharacterMesh);
			MorphAnimationManager::Sptr animator = character->Add<MorphAnimationManager>();

			std::vector<MeshResource::Sptr> RunAnim;
			std::vector<MeshResource::Sptr> JumpAnim;
			std::vector<MeshResource::Sptr> IdleAnim;

			RunAnim.push_back(DiscoBotMesh1);
			RunAnim.push_back(DiscoBotMesh2);
			RunAnim.push_back(DiscoBotMesh3);
			RunAnim.push_back(DiscoBotMesh4);
			RunAnim.push_back(DiscoBotMesh5);
			RunAnim.push_back(DiscoBotMesh6);
			RunAnim.push_back(DiscoBotMesh7);
			RunAnim.push_back(DiscoBotMesh8);
			RunAnim.push_back(DiscoBotMesh9);

			JumpAnim.push_back(BotJump1);
			JumpAnim.push_back(BotJump2);
			JumpAnim.push_back(BotJump3);
			JumpAnim.push_back(BotJump4);
			JumpAnim.push_back(BotJump5);
			JumpAnim.push_back(BotJump6);
			JumpAnim.push_back(BotJump7);

			IdleAnim.push_back(Idle1);
			IdleAnim.push_back(Idle2);
			IdleAnim.push_back(Idle3);

			animator->AddAnim(RunAnim, 0.05);
			animator->AddAnim(JumpAnim, 0.25);
			animator->AddAnim(IdleAnim, 0.25);
			animator->SetContinuity(true);
			animator->SetCurrentAnim(0);
		}

		GameObject::Sptr DiscoBall = scene->CreateGameObject("DiscoBall");
		{
			DiscoBall->SetPostion(glm::vec3(-10.270f, 5.710f, -1.0f));
			DiscoBall->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));
			DiscoBall->SetScale(glm::vec3(0.2f, 0.2f, 0.2f));

			RenderComponent::Sptr renderer = DiscoBall->Add<RenderComponent>();
			renderer->SetMesh(DiscoBallMesh);
			renderer->SetMaterial(DiscoBallMaterial);

			SeekBehaviour::Sptr seeking = DiscoBall->Add<SeekBehaviour>();
			seeking->seekTo(character);

			RigidBody::Sptr ballphysics = DiscoBall->Add<RigidBody>(RigidBodyType::Dynamic);
		}

		// Create some lights for our scene
	/*	GameObject::Sptr lightParent = scene->CreateGameObject("Lights");
		for (int ix = 0; ix < 50; ix++) {
			GameObject::Sptr light = scene->CreateGameObject("Light");
			light->SetPostion(glm::vec3(glm::diskRand(25.0f), 1.0f));
			lightParent->AddChild(light);

			Light::Sptr lightComponent = light->Add<Light>();
			lightComponent->SetColor(glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f)));
			lightComponent->SetRadius(glm::linearRand(0.1f, 10.0f));
			lightComponent->SetIntensity(glm::linearRand(1.0f, 2.0f));
		}*/

		// Create some lights for our scene
		GameObject::Sptr SingleLight = scene->CreateGameObject("SingleLight");


		GameObject::Sptr Slight = scene->CreateGameObject("Light");
		Slight->SetPostion(glm::vec3(0.0f, -3.0f, 0.0f));
		SingleLight->AddChild(Slight);

		Light::Sptr SlightComponent = Slight->Add<Light>();
		SlightComponent->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
		SlightComponent->SetRadius(30.0f);
		SlightComponent->SetIntensity(30.0f);

	/*	GameObject::Sptr shadowCaster = scene->CreateGameObject("Shadow Light");
		{
			// Set position in the scene
			shadowCaster->SetPostion(glm::vec3(3.0f, 3.0f, 5.0f)); 
			shadowCaster->LookAt(glm::vec3(0.0f));

			// Create and attach a renderer for the monkey
			ShadowCamera::Sptr shadowCam = shadowCaster->Add<ShadowCamera>();
			shadowCam->SetProjection(glm::perspective(glm::radians(120.0f), 1.0f, 0.1f, 100.0f));
		}*/


		GameObject::Sptr BeatBarVinyl = scene->CreateGameObject("Beat Bar Vinyl");
		{
			// Set position in the scene
			BeatBarVinyl->SetPostion(glm::vec3(-1.880f, 0.560f, -4.23f));
			BeatBarVinyl->SetRotation(glm::vec3(0.0f, 0.0f, 90.0f));
			BeatBarVinyl->SetScale(glm::vec3(0.75f, 0.75f, 0.1f));

			// Create and attach a renderer for the paddle
			RenderComponent::Sptr renderer = BeatBarVinyl->Add<RenderComponent>();
			renderer->SetMesh(BeatBarVinylMesh);
			renderer->SetMaterial(BeatBarVinylMaterial);

			BeatBarVinyl->Add<RotatingBehaviour>()->SetRotationSpeed(glm::vec3(0.f, 0.f, -360.f/2.4));

		}

		GameObject::Sptr BeatBarNeedle = scene->CreateGameObject("Beat Bar Needle");
		{
			// Set position in the scene
			BeatBarNeedle->SetPostion(glm::vec3(0.f, 0.89f, -4.140f));
			BeatBarNeedle->SetRotation(glm::vec3(90.0f, 0.0f, 158.0f));
			BeatBarNeedle->SetScale(glm::vec3(0.2f, 0.2f, 0.2f));

			// Create and attach a renderer for the paddle
			RenderComponent::Sptr renderer = BeatBarNeedle->Add<RenderComponent>();
			renderer->SetMesh(NeedleMesh);
			renderer->SetMaterial(BeatBarNeedleMaterial);

		}

		/////////////////////////// UI //////////////////////////////

		

		{//Pause Menu Block

			GameObject::Sptr PauseMenuDimBackground = scene->CreateGameObject("PauseMenu Dimmed Background");
			{//Dim BG
				RectTransform::Sptr transform = PauseMenuDimBackground->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = PauseMenuDimBackground->Add<GuiPanel>(0.5, 0.5, 1920, 1080);
				panel->SetTexture(TexDimmedBG);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;
			}

			GameObject::Sptr PauseMenuBackground = scene->CreateGameObject("PauseMenu Background");
			{//Background
				RectTransform::Sptr transform = PauseMenuBackground->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = PauseMenuBackground->Add<GuiPanel>(0.5, 0.5, 400, 750);
				panel->SetTexture(TexPauseMenu);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;
			}


			GameObject::Sptr ResumeButton = scene->CreateGameObject("PauseMenu Resume Button");
			{//Resume Button

				RectTransform::Sptr transform = ResumeButton->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = ResumeButton->Add<GuiPanel>(0.5, 0.28, 300, 150);
				panel->SetTexture(TexResumeButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

			}

			GameObject::Sptr OptionsButton = scene->CreateGameObject("PauseMenu Options Button");
			{//Options Button

				RectTransform::Sptr transform = OptionsButton->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = OptionsButton->Add<GuiPanel>(0.5, 0.43, 300, 150);
				panel->SetTexture(TexOptionsButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

			}

			GameObject::Sptr ResyncButton = scene->CreateGameObject("PauseMenu Resync Button");
			{//Resync Button

				RectTransform::Sptr transform = ResyncButton->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = ResyncButton->Add<GuiPanel>(0.5, 0.58, 300, 150);
				panel->SetTexture(TexResyncButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

			}

			GameObject::Sptr QuitButton = scene->CreateGameObject("PauseMenu Quit Button");
			{//Quit Button

				RectTransform::Sptr transform = QuitButton->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = QuitButton->Add<GuiPanel>(0.5, 0.73, 300, 150);
				panel->SetTexture(TexQuitButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

			}

			GameObject::Sptr PauseMenuNavigationIcon = scene->CreateGameObject("PauseMenu Navigation");
			{//Navigation Buttons

				RectTransform::Sptr transform = PauseMenuNavigationIcon->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = PauseMenuNavigationIcon->Add<GuiPanel>(0.9, 0.93, 400 * 0.75, 150 * 0.75);
				panel->SetTexture(TexNavigationUpDown);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

			}

			GameObject::Sptr PauseMenuParent = scene->CreateGameObject("PAUSE MENU PARENT");
			{
				PauseMenuParent->AddChild(PauseMenuDimBackground);
				PauseMenuParent->AddChild(PauseMenuBackground);
				PauseMenuParent->AddChild(OptionsButton);
				PauseMenuParent->AddChild(ResyncButton);
				PauseMenuParent->AddChild(ResumeButton);
				PauseMenuParent->AddChild(QuitButton);
				PauseMenuParent->AddChild(PauseMenuNavigationIcon);

				GuiPanel::Sptr panel = PauseMenuParent->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);

				panel->IsEnabled = false;
			}
		}

		{//Game Over Block
		
			GameObject::Sptr GameOverDimBackground = scene->CreateGameObject("GameOver Dimmed Background");
			{//Dim BG
		
				RectTransform::Sptr transform = GameOverDimBackground->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = GameOverDimBackground->Add<GuiPanel>(0.5, 0.5, 1920, 1080);
				panel->SetTexture(TexDimmedBG);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;
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
				panel->IsEnabled = false;
		
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
				panel->IsEnabled = false;
		
			}
		
		
			GameObject::Sptr GameOverQuit = scene->CreateGameObject("GameOver Quit Button");
			{//Quit
		
				RectTransform::Sptr transform = GameOverQuit->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });
		
				GuiPanel::Sptr panel = GameOverQuit->Add<GuiPanel>(0.35, 0.8, 300, 150);
				panel->SetTexture(TexQuitButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;
		
			}
		
			GameObject::Sptr GameOverContinue = scene->CreateGameObject("GameOver Continue Button");
			{//Continue Button
		
				RectTransform::Sptr transform = GameOverContinue->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });
		
				GuiPanel::Sptr panel = GameOverContinue->Add<GuiPanel>(0.65, 0.8, 300, 150);
				panel->SetTexture(TexContinueButton);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;
		
			}

			GameObject::Sptr GameOverNavigationIcon = scene->CreateGameObject("GameOver Navigation");
			{//Navigation Buttons

				RectTransform::Sptr transform = GameOverNavigationIcon->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });

				GuiPanel::Sptr panel = GameOverNavigationIcon->Add<GuiPanel>(0.9, 0.93, 400 * 0.75, 150 * 0.75);
				panel->SetTexture(TexNavigationLeftRight);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = false;

			}
		}
		
		//{//Tutorial Blocks
		//
		//	{//Movement
		//		GameObject::Sptr button = scene->CreateGameObject("Movement Tutorial");
		//
		//		RectTransform::Sptr transform = button->Add<RectTransform>();
		//		transform->SetPosition({ 0, 0 });
		//		transform->SetRotationDeg(0);
		//		transform->SetSize({ 700 * 0.75, 500 * 0.75 });
		//		transform->SetMin({ 0, 0 });
		//		transform->SetMax({ 700 * 0.75, 500 * 0.75 });
		//
		//		GuiPanel::Sptr panel = button->Add<GuiPanel>();
		//		panel->SetTexture(TexMovementTutorial);
		//		panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		//		panel->SetBorderRadius(0);
		//
		//
		//		transform->SetPosition({ (float)windowSize.x * 0.2, (float)windowSize.y * 0.2 });
		//
		//	}
		//
		//	{//Wall Jump
		//		GameObject::Sptr button = scene->CreateGameObject("Wall Jump Tutorial");
		//
		//		RectTransform::Sptr transform = button->Add<RectTransform>();
		//		transform->SetPosition({ 0, 0 });
		//		transform->SetRotationDeg(0);
		//		transform->SetSize({ 700 * 0.75, 500 * 0.75 });
		//		transform->SetMin({ 0, 0 });
		//		transform->SetMax({ 700 * 0.75, 500 * 0.75 });
		//
		//		GuiPanel::Sptr panel = button->Add<GuiPanel>();
		//		panel->SetTexture(TexWallJumpTutorial);
		//		panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		//		panel->SetBorderRadius(0);
		//
		//
		//		transform->SetPosition({ (float)windowSize.x * 0.2, (float)windowSize.y * 0.6 });
		//
		//	}
		//
		//	{//Beat Gem
		//		GameObject::Sptr button = scene->CreateGameObject("Beat Gem Tutorial");
		//
		//		RectTransform::Sptr transform = button->Add<RectTransform>();
		//		transform->SetPosition({ 0, 0 });
		//		transform->SetRotationDeg(0);
		//		transform->SetSize({ 700 * 0.75, 500 * 0.75 });
		//		transform->SetMin({ 0, 0 });
		//		transform->SetMax({ 700 * 0.75, 500 * 0.75 });
		//
		//		GuiPanel::Sptr panel = button->Add<GuiPanel>();
		//		panel->SetTexture(TexBeatGemTutorial);
		//		panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		//		panel->SetBorderRadius(0);
		//
		//
		//		transform->SetPosition({ (float)windowSize.x * 0.6, (float)windowSize.y * 0.2 });
		//
		//	}
		//
		//	{//Vinyls
		//		GameObject::Sptr button = scene->CreateGameObject("Vinyl Tutorial");
		//
		//		RectTransform::Sptr transform = button->Add<RectTransform>();
		//		transform->SetPosition({ 0, 0 });
		//		transform->SetRotationDeg(0);
		//		transform->SetSize({ 700 * 0.75, 500 * 0.75 });
		//		transform->SetMin({ 0, 0 });
		//		transform->SetMax({ 700 * 0.75, 500 * 0.75 });
		//
		//		GuiPanel::Sptr panel = button->Add<GuiPanel>();
		//		panel->SetTexture(TexVinylsTutorial);
		//		panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		//		panel->SetBorderRadius(0);
		//
		//
		//		transform->SetPosition({ (float)windowSize.x * 0.6, (float)windowSize.y * 0.6 });
		//
		//	}
		//}
		//
		//
		//{//HUD

			
			GameObject::Sptr ScoreBackground = scene->CreateGameObject("HUD Score Display");
			{//Score BG
				RectTransform::Sptr transform = ScoreBackground->Add<RectTransform>();
				transform->SetPosition({ 0, 0 });
				transform->SetRotationDeg(0);
				transform->SetSize({ 0, 0 });
				transform->SetMin({ 0, 0 });
		
				GuiPanel::Sptr panel = ScoreBackground->Add<GuiPanel>(0.13, 0.05, 250 * 2, 50 * 2);
				panel->SetTexture(TexScoreDisplay);
				panel->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				panel->SetBorderRadius(0);
				panel->IsEnabled = true;
		
			}
		
			GameObject::Sptr ScoreText = scene->CreateGameObject("HUD Score Text");
			{//Score Text
		
				RectTransform::Sptr transform = ScoreText->Add<RectTransform>();
				transform->SetRotationDeg(0);
				transform->SetMin({ 0, 0 });
		
				GuiText::Sptr text = ScoreText->Add<GuiText>(0.13, 0.07, 3.0);
				text->SetColor(glm::vec4(0.f));
				text->SetFont(FontVCR);
				text->SetText("0");
				text->IsEnabled = true;
		
			}

		//GameObject::Sptr particles = scene->CreateGameObject("Particles");
		//{
		//	ParticleSystem::Sptr particleManager = particles->Add<ParticleSystem>();
		//	particleManager->AddEmitter(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 10.0f), 10.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		//}

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
