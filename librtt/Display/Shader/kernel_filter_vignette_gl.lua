local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "vignette"

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

kernel.vertexData =
{
	{
		name = "radius",
		default = 0.15, -- ??
		-- min ?
		-- max ?
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	const P_COLOR float limit = 0.8;
	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;
	P_UV float dist = distance( texCoord, vec2( 0.5, 0.5 ) );
	texColor.rgb *= ( 1.0 - smoothstep( limit * v_UserData.x, limit, dist ) );
	return texColor;
}
]]

return kernel
