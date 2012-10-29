///////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL Image Copyright (c) 2008 - 2011 G-Truc Creation (www.g-truc.net)
///////////////////////////////////////////////////////////////////////////////////////////////////
// Created : 2008-12-19
// Updated : 2010-09-27
// Licence : This source is under MIT License
// File    : gli/gtx/fetch.inl
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace gli
{
	template <typename genType>
	inline genType texelFetch
	(
		texture2D const & Texture, 
		texture2D::dimensions_type const & TexCoord,
		texture2D::size_type const & Level
	)
	{
		assert(Texture.format() == R8U || Texture.format() == RG8U || Texture.format() == RGB8U || Texture.format() == RGBA8U);

		image::dimensions_type Dimensions = Texture[Level].dimensions();
		genType const * const Data = reinterpret_cast<genType const * const >(Texture[Level].data());

		return reinterpret_cast<genType const * const>(Data)[TexCoord.x + TexCoord.y * Dimensions.x];
	}

	template <typename genType>
	inline genType textureLod
	(
		texture2D const & Texture, 
		texture2D::texcoord_type const & TexCoord, 
		texture2D::size_type const & Level
	)
	{
		assert(Texture.format() == R8U || Texture.format() == RG8U || Texture.format() == RGB8U || Texture.format() == RGBA8U);

		image::dimensions_type Dimensions = Texture[Level].dimensions(); 
		genType const * const Data = reinterpret_cast<genType const * const>(Texture[Level].data());

		std::size_t s_below = std::size_t(glm::floor(TexCoord.s * float(Dimensions.x - 1)));
		std::size_t s_above = std::size_t(glm::ceil( TexCoord.s * float(Dimensions.x - 1)));
		std::size_t t_below = std::size_t(glm::floor(TexCoord.t * float(Dimensions.y - 1)));
		std::size_t t_above = std::size_t(glm::ceil( TexCoord.t * float(Dimensions.y - 1)));

		float s_step = 1.0f / float(Dimensions.x);
		float t_step = 1.0f / float(Dimensions.y);

		float s_below_normalized = s_below / float(Dimensions.x);
		float s_above_normalized = s_above / float(Dimensions.x);
		float t_below_normalized = t_below / float(Dimensions.y);
		float t_above_normalized = t_above / float(Dimensions.y);

		genType Value1 = reinterpret_cast<genType const * const>(Data)[s_below + t_below * Dimensions.x];
		genType Value2 = reinterpret_cast<genType const * const>(Data)[s_above + t_below * Dimensions.x];
		genType Value3 = reinterpret_cast<genType const * const>(Data)[s_above + t_above * Dimensions.x];
		genType Value4 = reinterpret_cast<genType const * const>(Data)[s_below + t_above * Dimensions.x];

		float BlendA = float(TexCoord.s - s_below_normalized) * float(Dimensions.x - 1);
		float BlendB = float(TexCoord.s - s_below_normalized) * float(Dimensions.x - 1);
		float BlendC = float(TexCoord.t - t_below_normalized) * float(Dimensions.y - 1);

		genType ValueA(glm::mix(Value1, Value2, BlendA));
		genType ValueB(glm::mix(Value4, Value3, BlendB));

		return genType(glm::mix(ValueA, ValueB, BlendC));
	}

	template <typename genType>
	void texelWrite
	(
		texture2D & Image,
		texture2D::dimensions_type const & Texcoord,
		texture2D::size_type const & Level,
		genType const & Color
	)
	{
		genType * Data = (genType*)Image[Level].data();
		std::size_t Index = Texcoord.x + Texcoord.y * Image[Level].dimensions().x;
		
		std::size_t Capacity = Image[Level].capacity();
		assert(Index < Capacity);

		*(Data + Index) = Color;
	}

}//namespace gli
