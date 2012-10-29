///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2012 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///
/// @ref core
/// @file gli/core/texture2d.inl
/// @date 2010-09-27 / 2012-10-16
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

namespace gli
{
	inline texture2D::texture2D() :
		Format(FORMAT_NULL),
		Offset(0)
	{}
/*
    inline texture2D::texture2D
    (
        size_type const & Levels,
        format_type const & Format,
        dimensions_type const & Dimensions,
        shared_ptr<detail::storage> const & Storage,
        size_type const & Offset
    ) :
        Storage(Storage),
        Format(Format),
        Offset(Offset)
    {}
*/
	inline texture2D::texture2D
	(
		size_type const & Levels,
		format_type const & Format,
		dimensions_type const & Dimensions
	) :
		Storage(shared_ptr<detail::storage>(new detail::storage(
			1, 1, Levels,
			detail::storage::dimensions3_type(Dimensions, glm::uint(1)),
			gli::bits_per_pixel(Format)))),
		Format(Format),
		Offset(0)
	{
		this->Images.resize(Levels);

		for(texture2D::size_type Level(0); Level < this->Images.size(); ++Level)
		{
			image::dimensions_type const Dimensions(this->Storage->dimensions(Level), 1);

			this->Images[Level] = image(
				this->Storage,
				this->Storage->linearAddressing(0, 0, Level) * gli::block_size(Format),
				Dimensions);
		}
	}

	inline texture2D::~texture2D()
	{}
 
	inline image & texture2D::operator[]
	(
		texture2D::size_type const & Level
	)
	{
		assert(Level < this->levels());

		return this->Images[Level];
	}

	inline image const & texture2D::operator[]
	(
		texture2D::size_type const & Level
	) const
	{
		assert(Level < this->levels());

		return this->Images[Level];
	}

	inline bool texture2D::empty() const
	{
		return this->Storage->empty();
	}

	inline texture2D::format_type texture2D::format() const
	{
		return this->Format;
	}

	inline texture2D::dimensions_type texture2D::dimensions() const
	{
		return texture2D::dimensions_type(this->Images[0].dimensions());
	}

	inline texture2D::size_type texture2D::levels() const
	{
		return this->Storage->levels();
	}

	inline texture2D::size_type texture2D::size() const
	{
		return this->Storage->memorySize();
	}

	inline texture2D::data_type* texture2D::data()
	{
		return this->Storage->data();
	}

	inline texture2D::data_type const * const texture2D::data() const
	{
		return this->Storage->data();
	}

/*
	template <typename genType>
	inline void texture2D::swizzle(glm::comp X, glm::comp Y, glm::comp Z, glm::comp W)
	{
		for(texture2D::level_type Level = 0; Level < this->levels(); ++Level)
		{
			genType * Data = reinterpret_cast<genType*>(this->Images[Level].data());
			texture2D::size_type Components = this->Images[Level].components();
			//gli::detail::getComponents(this->Images[Level].format());
			texture2D::size_type Size = (glm::compMul(this->Images[Level].dimensions()) * Components) / sizeof(genType);

			for(texture2D::size_type i = 0; i < Size; ++i)
			{
				genType Copy = Data[i];
				if(Components > 0)
					Data[i][0] = Copy[X];
				if(Components > 1)
					Data[i][1] = Copy[Y];
				if(Components > 2)
					Data[i][2] = Copy[Z];
				if(Components > 3)
					Data[i][3] = Copy[W];
			}
		}
	}
*/
/*
	template <typename T>
	inline T texture<T>::texture(float x, float y) const
	{
		size_type x_below = size_type(std::floor(x * (_width - 1)));
		size_type x_above = size_type(std::ceil(x * (_width - 1)));
		size_type y_below = size_type(std::floor(y * (_height - 1)));
		size_type y_above = size_type(std::ceil(y * (_height - 1)));

		float x_step = 1.0f / float(_width);
		float y_step = 1.0f / float(_height);

		float x_below_normalized = float(x_below) / float(_width - 1);
		float x_above_normalized = float(x_above) / float(_width - 1);
		float y_below_normalized = float(y_below) / float(_height - 1);
		float y_above_normalized = float(y_above) / float(_height - 1);
		
		T value1 = _data[x_below + y_below * _width];
		T value2 = _data[x_above + y_below * _width];
		T value3 = _data[x_above + y_above * _width];
		T value4 = _data[x_below + y_above * _width];

		T valueA = glm::mix(value1, value2, x - x_below_normalized);
		T valueB = glm::mix(value4, value3, x - x_below_normalized);
		T valueC = glm::mix(valueA, valueB, y - y_below_normalized);
		return valueC;
	}
*/
/*
	template <typename T>
	inline T texture(const texture2D<T>& Image2D, const glm::vec2& TexCoord)
	{
		texture2D<T>::size_type s_below = texture2D<T>::size_type(std::floor(TexCoord.s * (Image2D.width() - 1)));
		texture2D<T>::size_type s_above = texture2D<T>::size_type(std::ceil(TexCoord.s * (Image2D.width() - 1)));
		texture2D<T>::size_type t_below = texture2D<T>::size_type(std::floor(TexCoord.t * (Image2D.height() - 1)));
		texture2D<T>::size_type t_above = texture2D<T>::size_type(std::ceil(TexCoord.t * (Image2D.height() - 1)));

		glm::vec2::value_type s_step = 1.0f / glm::vec2::value_type(Image2D.width());
		glm::vec2::value_type t_step = 1.0f / glm::vec2::value_type(Image2D.height());

		glm::vec2::value_type s_below_normalized = glm::vec2::value_type(s_below) / glm::vec2::value_type(Image2D.width() - 1);
		glm::vec2::value_type s_above_normalized = glm::vec2::value_type(s_above) / glm::vec2::value_type(Image2D.width() - 1);
		glm::vec2::value_type t_below_normalized = glm::vec2::value_type(t_below) / glm::vec2::value_type(Image2D.height() - 1);
		glm::vec2::value_type t_above_normalized = glm::vec2::value_type(t_above) / glm::vec2::value_type(Image2D.height() - 1);
		
		T value1 = Image2D[s_below + t_below * Image2D.width()];
		T value2 = Image2D[s_above + t_below * Image2D.width()];
		T value3 = Image2D[s_above + t_above * Image2D.width()];
		T value4 = Image2D[s_below + t_above * Image2D.width()];

		T valueA = glm::mix(value1, value2, TexCoord.s - s_below_normalized);
		T valueB = glm::mix(value4, value3, TexCoord.s - s_below_normalized);
		T valueC = glm::mix(valueA, valueB, TexCoord.t - t_below_normalized);
		return valueC;
	}

	template <typename T>
	inline T textureNearest(const texture2D<T>& Image2D, const glm::vec2& TexCoord)
	{
		texture2D<T>::size_type s = texture2D<T>::size_type(glm::roundGTX(TexCoord.s * (Image2D.width() - 1)));
		texture2D<T>::size_type t = texture2D<T>::size_type(std::roundGTX(TexCoord.t * (Image2D.height() - 1)));

		return Image2D[s + t * Image2D.width()];
	}
*/

namespace wip
{
	////////////////
	// image
/*
	// 
	template
	<
		typename coordType
	>
	template
	<
		typename genType, 
		template <typename> class surface
	>
	typename texture2D<genType, surface>::value_type & 
	texture2D<genType, surface>::image_impl<coordType>::operator() 
	(
		coordType const & Coord
	)
	{
		
	}
*/
/*
	// 
	template
	<
		typename coordType
	>
	template
	<
		typename genType, 
		template <typename> class surface
	>
	typename texture2D<genType, surface>::value_type const & 
	texture2D<genType, surface>::image_impl::operator()
	(
		coordType const & Coord
	) const
	{
		return value_type(0);
	}
*/
/*
	//
	template
	<
		typename coordType
	>
	template
	<
		typename genType, 
		template <typename> class surface
	>
	void texture2D<genType, surface>::image_impl::operator()
	(
		coordType const & Coord
	) const
	{
		
	}
*/
	////
	//template
	//<
	//	typename genType, 
	//	template <typename> class surface
	//>
	//template
	//<
	//	typename coordType
	//>
	//typename texture2D<genType, surface>::value_type const & 
	//texture2D<genType, surface>::image_impl::operator()
	//(
	//	coordType const & Coord
	//) const
	//{
	//	return value_type(0);
	//}

	//////////////////
	//// texture2D

	//// 
	//template
	//<
	//	typename genType, 
	//	template <typename> class surface
	//>
	//typename texture2D<genType, surface>::level_type texture2D<genType, surface>::levels() const
	//{
	//	return this->Mipmaps.size();
	//}

	//// 
	//template
	//<
	//	typename genType, 
	//	template <typename> class surface
	//>
	//typename texture2D<genType, surface>::image & texture2D<genType, surface>::operator[] 
	//(
	//	typename texture2D<genType, surface>::level_type Level
	//)
	//{
	//	return this->Mipmaps[Level];
	//}

	//// 
	//template
	//<
	//	typename genType, 
	//	template <typename> class surface
	//>
	//typename texture2D<genType, surface>::image const & texture2D<genType, surface>::operator[] 
	//(
	//	typename texture2D<genType, surface>::level_type Level
	//) const
	//{
	//	return this->Mipmaps[Level];
	//}

}//namespace wip
}//namespace gli
