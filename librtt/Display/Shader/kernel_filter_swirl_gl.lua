local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "swirl"

kernel.vertexData =
{
	{
		name = "intensity",
		default = 0, -- ??
		-- min ?
		-- max ?
		index = 0, -- v_UserData.x
	},
}

--[[
	#if 0

		// Branching version.

		if( dist < radius ) 
		{
			P_UV float percent = (radius - dist) / radius;
			P_UV float theta = percent * percent * v_UserData.x * 8.0;
			P_UV float s = sin( theta );
			P_UV float c = cos( theta );
			uv = vec2( dot( uv, vec2( c, -s ) ), dot( uv, vec2( s, c ) ) );
		}

	#else

		// Branchless version.

	#endif
--]]

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    const P_UV float radius = 1.0;

    P_UV vec2 uv = texCoord - vec2( 0.5, 0.5 );
    P_UV float dist = length( uv );

	// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
	P_COLOR float useDistLessThanRadius = step( dist, radius );
	P_COLOR float useDistNotLessThanRadius = ( 1.0 - useDistLessThanRadius );

	P_UV float percent = (radius - dist) / radius;
	P_UV float theta = percent * percent * v_UserData.x * 8.0;
	P_UV float s = sin( theta );
	P_UV float c = cos( theta );

	P_UV vec2 resultDistLessThanRadius = ( vec2( dot( uv, vec2( c, -s ) ),
													dot( uv, vec2( s,  c ) ) ) +
											vec2( 0.5, 0.5 ) );
	P_UV vec2 resultDistNotLessThanRadius = texCoord;

    uv = ( ( useDistLessThanRadius * resultDistLessThanRadius ) +
			( useDistNotLessThanRadius * resultDistNotLessThanRadius ) );

    return texture2D( u_FillSampler0, uv ) * v_ColorScale;
}
]]

return kernel
