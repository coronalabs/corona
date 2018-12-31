local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "scatter"

kernel.vertexData =
{
	{
		name = "intensity",
		default = 0.5,
		min = 0,
		max = 1,
		index = 0, -- a_UserData.x
	},
}

kernel.vertex =
[[
varying P_UV float intensity;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	intensity = ( a_UserData.x * 0.25 );

	return position;
}
]]

kernel.fragment =
[[
varying P_UV float intensity;

// This function is duplicated in these:
//		kernel_filter_dissolve_gl.lua.
//		kernel_filter_random_gl.lua.
P_RANDOM float rand( in P_RANDOM vec2 seed )
{
	return fract( sin( dot( seed,
							vec2( 12.9898,
									78.233 ) ) ) * 43758.5453 );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
#if FRAGMENT_SHADER_SUPPORTS_HIGHP
	P_RANDOM vec2 rnd = vec2( rand( texCoord ),
								rand( texCoord.yx ));

	P_COLOR vec4 texColor = texture2D( u_FillSampler0,
										( texCoord + ( rnd * intensity ) ) );

	return texColor * v_ColorScale;
#else // FRAGMENT_SHADER_SUPPORTS_HIGHP
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	return texColor * v_ColorScale;
#endif // FRAGMENT_SHADER_SUPPORTS_HIGHP
}
]]

return kernel
