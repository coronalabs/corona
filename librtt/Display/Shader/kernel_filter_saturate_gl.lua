local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "saturate"

kernel.vertexData =
{
	{
		name = "intensity",
		default = 1,
		min = 0,
		max = 8,
		index = 0, -- v_UserData.x
	},
}

kernel.fragment =
[[
const P_COLOR vec3 kWeights = vec3( 0.2125, 0.7154, 0.0721 );

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR float intensity = v_UserData.x;

    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;

    P_COLOR float luminance = dot( texColor.rgb, kWeights );

    return vec4( mix( vec3( luminance ), texColor.rgb, intensity ), texColor.a );
}
]]

return kernel
