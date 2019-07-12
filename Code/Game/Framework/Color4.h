#pragma once


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Minimalistic floating point RGBA color class
*/
class Color4 final
{


//[-------------------------------------------------------]
//[ Public static data                                    ]
//[-------------------------------------------------------]
public:
	static const Color4 WHITE;	///< White (sRGB="1.0 1.0 1.0", Hex="#FFFFFF", alpha is one)
	static const Color4 BLACK;	///< Black (sRGB="0.0 0.0 0.0", Hex="#000000", alpha is one)
	static const Color4 RED;	///< Red   (sRGB="1.0 0.0 0.0", Hex="#FF0000", alpha is one)
	static const Color4 GREEN;	///< Green (sRGB="0.0 1.0 0.0", Hex="#008000", alpha is one)
	static const Color4 BLUE;	///< Blue  (sRGB="0.0 0.0 1.0", Hex="#0000FF", alpha is one)
	static const Color4 GRAY;	///< Gray  (sRGB="0.5 0.5 0.5", Hex="#808080", alpha is one)


//[-------------------------------------------------------]
//[ Public data                                           ]
//[-------------------------------------------------------]
public:
	union {
		float value[4];
		struct
		{
			float r, g, b, a;
		} rgba;
	};


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Default constructor
	*
	*  @note
	*    - Sets all components to zero
	*/
	inline Color4() :
		value { 0.0f, 0.0f, 0.0f, 0.0f }
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Copy constructor
	*
	*  @param[in] source
	*    Source to copy from
	*/
	inline explicit Color4(const Color4& source) :
		value { source[0], source[1], source[2], source[3] }
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Constructor
	*
	*  @param[in] r
	*    Value for the red component
	*  @param[in] g
	*    Value for the green component
	*  @param[in] b
	*    Value for the blue component
	*  @param[in] a
	*    Value for the alpha component
	*/
	inline Color4(float _r, float _g, float _b, float _a) :
		value { _r, _g, _b, _a }
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Destructor
	*/
	inline ~Color4()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Copy operator
	*
	*  @param[in] source
	*    Source to copy from
	*
	*  @return
	*    Reference to this instance
	*/
	inline Color4& operator =(const Color4& source)
	{
		value[0] = source.value[0];
		value[1] = source.value[1];
		value[2] = source.value[2];
		value[3] = source.value[3];
		return *this;
	}

	//[-------------------------------------------------------]
	//[ Assignment operators                                  ]
	//[-------------------------------------------------------]
	[[nodiscard]] inline operator float* ()
	{
		return &value[0];
	}

	[[nodiscard]] inline operator const float* () const
	{
		return &value[0];
	}


};
