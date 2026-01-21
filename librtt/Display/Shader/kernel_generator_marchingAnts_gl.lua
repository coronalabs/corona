local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "marchingAnts"

--kernel.isTimeDependent = true
kernel.timeTransform = {
	func = "modulo",

	-- the pattern is very regular, thus being able to mod the time
	-- the range is two bar lengths (on, then off), divided by speed
	-- the content scaling cancels out
	range = (6 + 6) / 16
}

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

-- This is based on "kernel_filter_stripesGenerator_gl.lua", without parameters.

kernel.vertex =
[[
varying P_UV float pixelSum1;
varying P_UV float pixelSumAll;
varying P_UV float pixelSum1_and_pixelOnPeriod2;
varying P_UV float pixelOffPeriod1_and_pixelOnPeriod2;

// Some Android devices AREN'T able to use "varying mat3".
varying P_UV vec3 transform0;
varying P_UV vec3 transform1;
varying P_UV vec3 transform2;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_UV float pixelOnPeriod1 = ( 6.0 / u_ContentScale.x );
	P_UV float pixelOffPeriod1 = ( 6.0 / u_ContentScale.x );
	P_UV float pixelOnPeriod2 = ( 6.0 / u_ContentScale.x );
	P_UV float pixelOffPeriod2 = ( 6.0 / u_ContentScale.x );

	P_UV float rotation_in_radians = radians( -45.0 );
	P_UV float translation = ( u_TotalTime * ( 16.0 / u_ContentScale.x ) );

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	pixelSum1 = ( pixelOnPeriod1 + pixelOffPeriod1 );
	P_UV float pixelSum2 = ( pixelOnPeriod2 + pixelOffPeriod2 );
	pixelSumAll = ( pixelSum1 + pixelSum2 );
	pixelSum1_and_pixelOnPeriod2 = ( pixelSum1 + pixelOnPeriod2 );
	pixelOffPeriod1_and_pixelOnPeriod2 = ( pixelOffPeriod1 + pixelOnPeriod2 );

	// The Y translation component is 0.0 because we ONLY
	// support generating stripes along the X axis.
	{
		P_UV float s = sin( rotation_in_radians );
		P_UV float c = cos( rotation_in_radians );

		transform0 = vec3( c, ( - s ), 0.0 );
		transform1 = vec3( s, c, 0.0 );
		transform2 = vec3( translation, 0.0, 1.0 );
	}

	return position;
}
]]

kernel.fragment =
[[
varying P_UV float pixelSum1;
varying P_UV float pixelSumAll;
varying P_UV float pixelSum1_and_pixelOnPeriod2;
varying P_UV float pixelOffPeriod1_and_pixelOnPeriod2;

// Some Android devices AREN'T able to use "varying mat3".
varying P_UV vec3 transform0;
varying P_UV vec3 transform1;
varying P_UV vec3 transform2;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	// We CAN'T use "texCoord" here because it DOESN'T provide useful texture
	// coordinates for the effect we're trying to achieve. The simplest
	// solution is to use "gl_FragCoord" instead.
	//
	// "gl_FragCoord" is in PIXELS.
	//
	// Apply the rotation and translation parameters.
	P_UV mat3 transform;
	transform[ 0 ] = transform0;
	transform[ 1 ] = transform1;
	transform[ 2 ] = transform2;

	P_UV vec2 tc = ( transform * vec3( gl_FragCoord.xy, 1.0 ) ).xy;

	// Get the position wrapped to the total length of the pattern.
	P_UV float pixel_x = mod( tc.x, pixelSumAll );

	P_UV float gray;

	#if 0 // This is slow and provided for readability ONLY.

		if( pixel_x < pixelSum1 )
		{
			// First period.

			// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
			gray = step( pixel_x, pixelOnPeriod1 );
		}
		else
		{
			// Second period.

			// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
			gray = step( ( pixel_x - pixelSum1 ), pixelOnPeriod2 );
		}

	#else

		// Select first (lower) or second (upper) "on" period as the threshold for the pixel.
		// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
		P_UV float threshold_delta = ( pixelOffPeriod1_and_pixelOnPeriod2 * step( pixel_x, pixelSum1 ) );

		// pixelSum1_and_pixelOnPeriod2 is the upper threshold.
		P_UV float threshold  = ( pixelSum1_and_pixelOnPeriod2 - threshold_delta );

		// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
		gray = step( pixel_x, threshold );

	#endif

	return ( vec4( vec3( gray ), 1.0 ) * v_ColorScale );
}
]]

return kernel
