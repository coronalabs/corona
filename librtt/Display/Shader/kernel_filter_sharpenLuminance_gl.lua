local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "sharpenLuminance"

kernel.vertexData =
{
	{
		name = "sharpness",
		default = 0,
		min = 0,
		max = 1,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR float unit_sharpness = v_UserData.x;
	P_COLOR float sharpness_factor = ( 10.0 + ( 40.0 * unit_sharpness ) );

	P_COLOR vec4 color = texture2D( u_FillSampler0, texCoord );
	P_COLOR float original_alpha = color.a;

	color -= texture2D( u_FillSampler0, texCoord + 0.0001 ) * sharpness_factor;
	color += texture2D( u_FillSampler0, texCoord - 0.0001 ) * sharpness_factor;

	color.a = original_alpha;

	return color;
}
]]

return kernel
