local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "monotone"

kernel.vertexData =
{
	{
		name = "r",
		default = 0,
		min = 0,
		max = 1,
		index = 0, -- v_UserData.x
	},
	{
		name = "g",
		default = 0,
		min = 0,
		max = 1,
		index = 1, -- v_UserData.y
	},
	{
		name = "b",
		default = 0,
		min = 0,
		max = 1,
		index = 2, -- v_UserData.z
	},
	{
		name = "a",
		default = 1,
		min = 0,
		max = 1,
		index = 3, -- v_UserData.w
	},
}

kernel.fragment =
[[
const P_COLOR vec3 kWeights = vec3( 0.2125, 0.7154, 0.0721 );

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;

    P_COLOR float luminance = dot( texColor.rgb, kWeights );
	
    P_COLOR vec4 desat = vec4( vec3( luminance ), 1.0 );
	
	// overlay
    P_COLOR vec4 outputColor = vec4(
        (desat.r < 0.5 ? (2.0 * desat.r * v_UserData.r) : (1.0 - 2.0 * (1.0 - desat.r) * (1.0 - v_UserData.r))),
        (desat.g < 0.5 ? (2.0 * desat.g * v_UserData.g) : (1.0 - 2.0 * (1.0 - desat.g) * (1.0 - v_UserData.g))),
        (desat.b < 0.5 ? (2.0 * desat.b * v_UserData.b) : (1.0 - 2.0 * (1.0 - desat.b) * (1.0 - v_UserData.b))),
        1.0
    );

	P_COLOR float intensity = v_UserData.a;

	return vec4( mix(texColor.rgb, outputColor.rgb, intensity), texColor.a);
}
]]

return kernel
