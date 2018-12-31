local kernel = {}

kernel.language = "glsl"

kernel.category = "composite"

kernel.name = "reflect"

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

P_COLOR vec4 Reflect( in P_COLOR vec4 base, in P_COLOR vec4 blend )
{
    // Luminance of blend
    P_COLOR float luminance = dot( blend, kWeights );

	#if 0

		// Branching version.

	    P_COLOR vec4 result;
	    if( luminance == 1.0 )
	    {
	        result = blend;
	    }
	    else
	    {
	        result = min( base * base / ( 1.0 - blend ), 1.0 );
	    }

	    return result;

	#else

		// Branchless version.

		// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
		P_COLOR float luminanceIsOne = step( luminance, 1.0 );
		P_COLOR float luminanceIsNotOne = ( 1.0 - luminanceIsOne );

		P_COLOR vec4 resultLuminanceIsNotOne = min( base * base / ( 1.0 - blend ), 1.0 );

		return ( ( luminanceIsOne * blend ) +
					( luminanceIsNotOne * resultLuminanceIsNotOne ) );

	#endif
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 base = texture2D( u_FillSampler0, texCoord );
    P_COLOR vec4 blend = texture2D( u_FillSampler1, texCoord );

    P_COLOR vec4 result = Reflect( base, blend );

    return mix( base, result, v_UserData.x ) * v_ColorScale;
}
]]

return kernel
