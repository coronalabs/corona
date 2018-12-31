local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "linearWipe"

kernel.uniformData =
{
	{
		name = "direction",
		default = { 1, 0 },
		min = { 0, 0 },
		max = { 1, 1 },
		type="vec2",
		index = 0, -- u_UserData0
	},
	{
		name = "smoothness",
		default = 0,
		min = 0,
		max = 1,
		type="scalar",
		index = 1, -- u_UserData1
	},
	{
		name = "progress",
		default = 0,
		min = 0,
		max = 1,
		type="scalar",
		index = 2, -- u_UserData2
	},
}

kernel.vertex =
[[
uniform P_NORMAL vec2 u_UserData0; // direction
uniform P_UV float u_UserData1; // unitSmoothness
uniform P_UV float u_UserData2; // unitProgress

varying P_POSITION vec2 fromPos;

varying P_NORMAL vec2 N;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	N = normalize( u_UserData0 );

	// unitSmoothness is [ 0.0 .. 1.0 ].
	//
	//      0.0: hard.
	//      1.0: smooth.
	P_UV float unitSmoothness = u_UserData1;

	// unitProgress is [ 0.0 .. 1.0 ].
	//
	//      0.0: start.
	//      1.0: end.
	P_UV float unitProgress = u_UserData2;

	// The start and end point of V.
	P_UV vec2 A;
	P_UV vec2 B;

	// Branching sucks, but this is the simplest solution we found.
	if( ( N.x >= 0.0 ) &&
		( N.y >= 0.0 ) )
	{
		// Upper left, towards lower right.

		A = vec2( 0.0, 0.0 );
		B = vec2( 1.0, 1.0 );
	}
	else if( ( N.x <= 0.0 ) &&
				( N.y >= 0.0 ) )
	{
		// Upper right, towards lower left.

		A = vec2( 1.0, 0.0 );
		B = vec2( 0.0, 1.0 );
	}
	else if( ( N.x <= 0.0 ) &&
				( N.y <= 0.0 ) )
	{
		// Lower right, towards upper left.

		A = vec2( 1.0, 1.0 );
		B = vec2( 0.0, 0.0 );
	}
	else // if( ( N.x >= 0.0 ) && ( N.y <= 0.0 ) )
	{
		// Lower left, towards upper right.

		A = vec2( 0.0, 1.0 );
		B = vec2( 1.0, 0.0 );
	}

	P_UV vec2 V = ( B - A );

	P_UV float distance_to_cover = ( abs( dot( V, N ) ) + unitSmoothness );

	// Get the fromPos.
	P_UV float fromPos_progress = ( ( unitProgress * distance_to_cover ) - unitSmoothness );
	fromPos = ( A + ( N * fromPos_progress ) );

	return position;
}
]]

kernel.fragment =
[[
uniform P_NORMAL vec2 u_UserData0; // direction
uniform P_UV float u_UserData1; // unitSmoothness
uniform P_UV float u_UserData2; // unitProgress

varying P_POSITION vec2 fromPos;

varying P_NORMAL vec2 N;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	// unitSmoothness is [ 0.0 .. 1.0 ].
	//
	//      0: hard.
	//      1: smooth.
	P_UV float unitSmoothness = u_UserData1;

	// "W" : A vector from the "fromPos" to the current fragment.
	//WE *SHOULD* BE ABLE TO MOVE THIS TO THE VERTEX SHADER!!!
	P_UV vec2 W = ( texCoord - fromPos );

	// "d" : The progress of "W" along "N".
	P_UV float d = dot( W, N );

	// Keep "d" within reasonable bounds.
	d = clamp( d, 0.0, unitSmoothness );

	// "progress" : The unitized progress of "d" along "V".
	P_UV float progress = ( d / unitSmoothness );

	// Get the colors to modulate.
	// We want to use texCoord instead of "translated_texCoord" here because
	// we want to sample the texture at its unmodified coordinates.
	// "translated_texCoord" is only useful to determine the transparency.
	P_COLOR vec4 color = ( texture2D( u_FillSampler0, texCoord ) * v_ColorScale );

	#if 0 // For debugging ONLY.

		// Blend with a plain color only.
		// Diregard the texture color sampled above.
		color = v_ColorScale;

	#elif 0 // For debugging ONLY.

		// Use debug colors only.
		if( progress <= 0.0 )
		{
			return vec4( 1.0, 0.0, 0.0, 1.0 );
		}
		else if( progress >= 1.0 )
		{
			return vec4( 0.0, 1.0, 0.0, 1.0 );
		}
		else // if( ( progress > 0.0 ) && ( progress < 1.0 ) )
		{
			return vec4( 0.0, 0.0, 1.0, 1.0 );
		}

	#endif

	// Linear interpolation between colors.
	// This multiplies-in the alpha.
	return mix( color,
				vec4( 0.0 ),
				progress );
}
]]

return kernel
