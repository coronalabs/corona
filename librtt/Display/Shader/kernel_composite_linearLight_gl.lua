local kernel = {}

kernel.language = "glsl"

kernel.category = "composite"

kernel.name = "linearLight"

kernel.vertexData =
{
    {
        name = "alpha",
        default = 1,
        min = 0,
        max = 1,
        index = 0, -- v_UserData.x
    },
}

kernel.fragment =
[[
const P_COLOR vec4 kWeights = vec4( 0.2125, 0.7154, 0.0721, 1.0 );

P_COLOR vec4 Add( in P_COLOR vec4 base, in P_COLOR vec4 blend )
{
    return base + blend;
}

P_COLOR vec4 Subtract( in P_COLOR vec4 base, in P_COLOR vec4 blend )
{
    return max( base + blend - 1.0, 0.0 );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 base = texture2D( u_FillSampler0, texCoord );
    P_COLOR vec4 blend = texture2D( u_FillSampler1, texCoord );

    // Luminance of blend
    P_COLOR float luminance = dot( blend, kWeights );

	#if 0

		// Branching version.

		P_COLOR vec4 result;
	    if ( luminance < 0.5 )
	    {
	        result = Subtract( base, 2.0 * blend );
	    }
	    else
	    {
	        result = Add( base, 2.0 * ( blend - 0.5 ) );
	    }

	#else

		// Branchless version.

		// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
		P_COLOR float useLuminanceLow = step( luminance, 0.5 );
		P_COLOR float useLuminanceHigh = ( 1.0 - useLuminanceLow );

	    P_COLOR vec4 resultLuminanceLow = Subtract( base, 2.0 * blend );
	    P_COLOR vec4 resultLuminanceHigh = Add( base, 2.0 * ( blend - 0.5 ) );

	    P_COLOR vec4 result = ( ( useLuminanceLow * resultLuminanceLow ) +
								( useLuminanceHigh * resultLuminanceHigh ) );

	#endif

    return mix( base, result, v_UserData.x ) * v_ColorScale;
}
]]

return kernel
