#pragma once
#include <memory>

#include "Device/IDevice.h"
#include "Singleton.h"
#include "Resource/ResourceCache.h"
#include "Graphics/ModelResource.h"
#include "Utils/DirectXHelper.h"

#if ME_ENABLE_RENDERDOC
#include "Debug/RenderDocManager.h"
#endif
#include <d3d11.h>
#include <DirectXMath.h>
#include <queue>
#include "Math/Vector2.h"
#include <functional>
#include "RenderCommands.h"
#include "Camera/CameraData.h"
#include "GeometricPrimitive.h"
#include <PrimitiveBatch.h>
#include "Graphics/ShaderStructures.h"
#include <SpriteFont.h>
#include <SpriteBatch.h>
#include "Events/Event.h"

namespace Moonlight
{
	class PickingEvent
		: public Event<PickingEvent>
	{
	public:
		int Id = 0;
	};

	class LegacyRenderer
	{
	public:
		enum class RenderPassType
		{
			Differed,
			Forward
		};
		RenderPassType PassType = RenderPassType::Differed;
		void UpdateMatrix(unsigned int Id, DirectX::SimpleMath::Matrix NewTransform);
		void UpdateMeshMatrix(unsigned int Id, DirectX::SimpleMath::Matrix NewTransform);
		void UpdateCamera(unsigned int Id, CameraData& NewCommand);
		Moonlight::CameraData& GetCamera(unsigned int Id);
	public:
		LegacyRenderer();
		virtual ~LegacyRenderer() final;

		void Init();

		void SetWindow();
		void RegisterDeviceNotify(IDeviceNotify* deviceNotify);

		LegacyDX11Device& GetDevice();

		void Update(float dt);
		void Render(std::function<void()> func, std::function<void()> uiRender, const CameraData& editorCamera);

		void DrawScene(ID3D11DeviceContext3* context, ModelViewProjectionConstantBuffer& constantBufferSceneData, const CameraData& data, FrameBuffer* ViewRTT);
		void DrawDepthOnlyScene(ID3D11DeviceContext3* context, DepthPassBuffer& constantBufferSceneData, FrameBuffer* ViewRTT);
		void DrawPickingTexture(ID3D11DeviceContext3* context, PickingConstantBuffer& constantBufferSceneData, const CameraData& camera, FrameBuffer* ViewRTT);

		void ReleaseDeviceDependentResources();
		void CreateDeviceDependentResources();
		void InitD2DScreenTexture();

		unsigned int PushDebugCollider(const DebugColliderCommand& model);
		bool PopDebugCollider(unsigned int id);

		void ClearDebugColliders();

		unsigned int PushLight(const LightCommand& NewLight);
		bool PopLight(unsigned int id);

		void ClearLights();

		void WindowResized(const Vector2& NewSize);

		void PickScene(const Vector2& Pos);

		//class RenderTexture* GameViewRTT = nullptr;
		FrameBuffer* SceneViewRTT = nullptr;
		FrameBuffer* GameViewRTT = nullptr;
		LightCommand Sunlight;
		LightingPassConstantBuffer LightingPassBuffer;

		ShaderProgram m_tonemapProgram;
		ShaderProgram m_dofProgram;
		ShaderProgram m_depthProgram;
		ShaderProgram m_pickingShader;
		ShaderProgram m_lightingProgram;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_defaultSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_computeSampler;
	private:
		class MeshData* PlaneMesh = nullptr;

		class LegacyDX11Device* m_device = nullptr;
		std::vector<Vertex> m_planeVerticies;
		std::vector<uint16_t> m_planeIndicies;
		void ResizeBuffers();
		void SaveTextureToBmp(PCWSTR FileName, ID3D11Texture2D* Texture, const CameraData& camera, const Vector2& Pos);
#if ME_DIRECTX
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_perFrameBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_depthPassBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pickingBuffer;
		ModelViewProjectionConstantBuffer m_constantBufferData;
		ModelViewProjectionConstantBuffer m_constantBufferSceneData;
		std::unique_ptr<DirectX::PrimitiveBatch<VertexPositionTexCoord>> primitiveBatch;
#endif
		bool m_pickingRequested = false;
		Vector2 pickingLocation;
		// Text
		std::unique_ptr<DirectX::SpriteFont> m_font;
		DirectX::SimpleMath::Vector2 m_fontPos;
		std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

		// Commands
		std::vector<DebugColliderCommand> DebugColliders;
		std::queue<unsigned int> FreeDebugColliderCommandIndicies;

		std::vector<LightCommand> Lights;
		std::queue<unsigned int> FreeLightCommandIndicies;

		std::vector<MeshCommand> Meshes;
		std::queue<unsigned int> FreeMeshCommandIndicies;

		std::vector<CameraData> Cameras;
		std::queue<unsigned int> FreeCameraCommandIndicies;

#if ME_ENABLE_RENDERDOC
		RenderDocManager* RenderDoc;
#endif
	public:
		unsigned int PushMesh(Moonlight::MeshCommand command);
		void PopMesh(unsigned int Id);
		void ClearMeshes();

		unsigned int PushCamera(Moonlight::CameraData& command);
		void PopCamera(unsigned int Id);
		void ClearUIText();

		std::unique_ptr<DirectX::GeometricPrimitive> shape;
	};
}