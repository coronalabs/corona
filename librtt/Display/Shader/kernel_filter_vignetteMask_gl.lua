local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "vignetteMask"

kernel.vertexData =
{
	{
		name = "innerRadius",
		default = 0.25, -- ??
		-- min ?
		-- max ?
		index = 0, -- v_UserData.x
	},
	{
		name = "outerRadius",
		default = 0.8, -- ??
		-- min ?
		-- max ?
		index = 1, -- v_UserData.y
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV float lower_bound = v_UserData.x;
	P_UV float upper_bound = v_UserData.y;

	P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;

	P_UV float dist = distance( texCoord, vec2( 0.5, 0.5 ) );

	P_COLOR float stepping = smoothstep( lower_bound,
											upper_bound,
											dist );

	texColor.rgba *= stepping;

	return texColor;
}
]]

return kernel
