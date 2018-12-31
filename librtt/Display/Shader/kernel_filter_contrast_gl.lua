local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "contrast"

kernel.vertexData =
{
	{
		name = "contrast",
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
    P_COLOR float contrast = v_UserData.x;

    const P_COLOR vec3 avgLuminance = vec3( 0.5, 0.5, 0.5 );

    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;
    texColor.rgb = mix( avgLuminance, texColor.rgb, contrast);

    return texColor;
}
]]

return kernel
