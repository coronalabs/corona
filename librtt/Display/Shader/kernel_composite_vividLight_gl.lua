local kernel = {}

kernel.language = "glsl"

kernel.category = "composite"

kernel.name = "vividLight"

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

--[[
	#if 0

		// Branching version.

	    if( luminance < 0.5 )
	    {
	        result = ColorBurn( base, 2.0 * blend );
	    }
	    else
	    {
	        result = ColorDodge( base, 2.0 * ( blend - 0.5 ) );
	    }

	#else

		// Branchless version.

	#endif
--]]

kernel.fragment =
[[
const P_COLOR vec4 kWeights = vec4( 0.2125, 0.7154, 0.0721, 1.0 );

P_COLOR vec4 ColorBurn( in P_COLOR vec4 base, in P_COLOR vec4 blend )
{
    // TODO: Test for ( blend == 0.0 )???
    P_COLOR vec4 result = 1.0 - ( 1.0 - base ) / blend;
    return result;
}

P_COLOR vec4 ColorDodge( in P_COLOR vec4 base, in P_COLOR vec4 blend )
{
    // TODO: Test for ( blend == 1.0 )???
    P_COLOR vec4 result = base / ( 1.0 - blend );
    return result;
}

P_COLOR vec4 VividLight( in P_COLOR vec4 base, in P_COLOR vec4 blend )
{
	// Luminance of blend
	P_COLOR float luminance = dot( blend, kWeights );

	// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
	P_COLOR float useLuminanceLow = step( luminance, 0.5 );
	P_COLOR float useLuminanceHigh = ( 1.0 - useLuminanceLow );

	P_COLOR vec4 resultLuminanceLow = ColorBurn( base, 2.0 * blend );
	P_COLOR vec4 resultLuminanceHigh = ColorDodge( base, 2.0 * ( blend - 0.5 ) );

	return ( ( useLuminanceLow * resultLuminanceLow ) +
				( useLuminanceHigh * resultLuminanceHigh ) );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 base = texture2D( u_FillSampler0, texCoord );
    P_COLOR vec4 blend = texture2D( u_FillSampler1, texCoord );

    P_COLOR vec4 result = VividLight( base, blend );

    return mix( base, result, v_UserData.x ) * v_ColorScale;
}
]]

return kernel
