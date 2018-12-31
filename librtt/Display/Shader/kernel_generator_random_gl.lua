local kernel = {}

kernel.language = "glsl"

kernel.category = "generator"

kernel.name = "random"

kernel.isTimeDependent = true

kernel.fragment =
[[
// This function is duplicated in these:
//		kernel_filter_dissolve_gl.lua.
//		kernel_filter_scatter_gl.lua.
P_RANDOM float rand( P_RANDOM vec2 seed )
{
	return fract( sin( dot( seed,
							vec2( 12.9898,
									78.233 ) ) ) * 43758.5453 );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
#if FRAGMENT_SHADER_SUPPORTS_HIGHP
	P_RANDOM float time = fract( u_TotalTime );

	P_RANDOM float v0 = rand( vec2( ( time + texCoord.x ),
									( time + texCoord.y ) ) );

	#if 0

		// Grayscale.

		return ( vec4( vec3( v0 ), 1.0 ) * v_ColorScale );

	#else

		// Any color.

		P_RANDOM float v1 = rand( vec2( ( time + texCoord.x + u_TexelSize.x ),
									( time + texCoord.y ) ) );

		P_RANDOM float v2 = rand( vec2( ( time + texCoord.x ),
									( time + texCoord.y + u_TexelSize.y ) ) );

		return ( vec4( v0, v1, v2, 1.0 ) * v_ColorScale );

	#endif
#else // FRAGMENT_SHADER_SUPPORTS_HIGHP
	return vec4( 0 );
#endif // FRAGMENT_SHADER_SUPPORTS_HIGHP
}
]]

return kernel
