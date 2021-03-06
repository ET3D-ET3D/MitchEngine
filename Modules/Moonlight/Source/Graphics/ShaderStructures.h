﻿#pragma once
#include <DirectXMath.h>
#include "RenderCommands.h"

#if ME_DIRECTX
namespace Moonlight
{
	struct PerFrameConstantBuffer
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT2 ViewportSize;
		DirectX::XMFLOAT2 _Padding;
	};
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 modelInv;
		DirectX::XMFLOAT2 Tiling;
		BOOL HasNormalMap;
		BOOL HasAlphaMap;
		BOOL HasSpecMap;
		DirectX::XMFLOAT3 DiffuseColor;
		DirectX::XMFLOAT4 padding2;
	};

	struct PickingConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		FLOAT id;
		DirectX::XMFLOAT3 padding;
	};

	struct DepthPassBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 cameraMatrix;
	};

	struct LightingPassConstantBuffer
	{
		LightCommand Light;
		DirectX::XMMATRIX LightSpaceMatrix;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};
	struct VertexPositionTexCoord
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TexCoord;
	};
	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TextureCoord;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT3 BiTangent;
	};

	struct ShaderProgram
	{
		Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
	};

	struct ComputeProgram
	{
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> ComputeShader;
	};
}
#endif