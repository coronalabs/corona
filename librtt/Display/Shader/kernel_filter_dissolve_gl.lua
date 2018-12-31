local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "dissolve"

kernel.vertexData =
{
	{
		name = "threshold",
		default = 1, -- ??
		-- min ?
		-- max ?
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
// This function is duplicated in these:
//		kernel_filter_random_gl.lua.
//		kernel_filter_scatter_gl.lua.
P_RANDOM float rand( in P_RANDOM vec2 seed )
{
	return fract( sin( dot( seed,
							vec2( 12.9898,
									78.233 ) ) ) * 43758.5453 );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
#if FRAGMENT_SHADER_SUPPORTS_HIGHP
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
	P_RANDOM float noise = step( rand( texCoord ), v_UserData.x );

	texColor *= noise;

	return texColor * v_ColorScale;
#else // FRAGMENT_SHADER_SUPPORTS_HIGHP
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	return texColor * v_ColorScale;
#endif // FRAGMENT_SHADER_SUPPORTS_HIGHP
}
]]

return kernel
