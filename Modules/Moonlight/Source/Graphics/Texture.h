#pragma once
#include <string>
#include "Dementia.h"
#include "Resource/Resource.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace Moonlight { struct FrameBuffer; }

namespace Moonlight
{
	enum TextureType
	{
		Diffuse = 0,
		Normal,
		Specular,
		Height,
		Opacity,
		Count
	};

	enum WrapMode
	{
		Clamp,
		Decal,
		Wrap,
		Mirror
	};

	class Texture
		: public Resource
	{
	public:
		unsigned int Id;
		TextureType Type;

		int mWidth;
		int mHeight;
		int mChannels;

		Texture(const Path& InFilePath, WrapMode mode = WrapMode::Wrap);
		Texture(FrameBuffer* InFilePath, WrapMode mode = WrapMode::Wrap);
		~Texture();

		void UpdateBuffer(FrameBuffer* NewBuffer);

		template<typename T> static constexpr T NumMipmapLevels(T width, T height)
		{
			T levels = 1;
			while ((width | height) >> levels) {
				++levels;
			}
			return levels;
		}

		// Textures should not be copied around in memory
		ME_NONCOPYABLE(Texture);

		ID3D11ShaderResourceView* ShaderResourceView = nullptr;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState;
		ID3D11Resource* resource = nullptr;

		static std::string ToString(TextureType type);
	};
}