local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "stripes"

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

-- This is the model for "kernel_filter_marchingAnts_gl.lua".

kernel.uniformData =
{
	{
		-- x is pixelOnPeriod1.
		-- y is pixelOffPeriod1.
		-- z is pixelOnPeriod2.
		-- w is pixelOffPeriod2.
		name = "periods",
		default = { 1, 1, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 99999, 99999, 99999, 99999 },
		type="vec4",
		index = 0, -- u_UserData0
	},
	{
		name = "angle",
		default = 0,
		min = 0,
		max = 360,
		type="scalar",
		index = 1, -- u_UserData1
	},
	{
		name = "translation",
		default = 0,
		min = -99999,
		max = 99999,
		type="scalar",
		index = 2, -- u_UserData2
	},
}

kernel.vertex =
[[
uniform P_UV vec4 u_UserData0; // periods
uniform P_UV float u_UserData1; // angle
uniform P_UV float u_UserData2; // translation

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
	P_UV float pixelOnPeriod1 = ( u_UserData0.x / u_ContentScale.x );
	P_UV float pixelOffPeriod1 = ( u_UserData0.y / u_ContentScale.x );
	P_UV float pixelOnPeriod2 = ( u_UserData0.z / u_ContentScale.x );
	P_UV float pixelOffPeriod2 = ( u_UserData0.w / u_ContentScale.x );

	P_UV float rotation_in_radians = radians( u_UserData1 );
	P_UV float translation = ( u_UserData2 / u_ContentScale.x );

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
uniform P_UV vec4 u_UserData0; // periods
uniform P_UV float u_UserData1; // angle
uniform P_UV float u_UserData2; // translation

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

	P_COLOR float gray;

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
