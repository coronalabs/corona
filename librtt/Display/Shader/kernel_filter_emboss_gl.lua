local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "emboss"

kernel.vertexData =
{
	{
		name = "intensity",
		default = 1,
		min = 0,
		max = 4,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR vec4 sample0 = texture2D( u_FillSampler0, texCoord - u_TexelSize.xy );
	P_COLOR vec4 sample1 = texture2D( u_FillSampler0, texCoord + u_TexelSize.xy );

	P_COLOR vec4 result = vec4( 0.5, 0.5, 0.5, ( ( sample0.a + sample1.a ) * 0.5 ) );
	result.rgb -= sample0.rgb * 5.0 * v_UserData.x;
	result.rgb += sample1.rgb * 5.0 * v_UserData.x;
	result.rgb = vec3( ( result.r + result.g + result.b ) * ( 1.0 / 3.0 ) );

	// Pre-multiply alpha.
	result.rgb *= result.a;

	return result * v_ColorScale;
}
]]

return kernel
