local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "sepia"

kernel.vertexData =
{
	{
		name = "intensity",
		default = 1,
		min = 0,
		max = 1,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	const P_COLOR vec3 LUMINANCE_WEIGHTS = vec3( 0.22, 0.707, 0.071 );
	const P_COLOR vec3 lightColor = vec3( 1.0, 0.9, 0.5 );
	const P_COLOR vec3 darkColor = vec3( 0.2, 0.05, 0.0 );

	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;
	P_COLOR float luminance = dot( LUMINANCE_WEIGHTS, texColor.xyz );
	P_COLOR vec3 sepia = lightColor * luminance + ( -darkColor * luminance + darkColor );

	P_COLOR vec3 result = mix( texColor.rgb, sepia.rgb, v_UserData.x );

	// Pre-multiply alpha.
	result.rgb *= texColor.a;

	return vec4( result.rgb, texColor.a );
}
]]

return kernel
