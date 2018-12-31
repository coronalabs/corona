local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "duotone"

kernel.uniformData =
{
	{
		name = "darkColor",
		default = { 0.0, 0.0, 0.5, 1.0 },
		type = "vec4",
		index = 0, -- u_UserData0
	},
	{
		name = "lightColor",
		default = { 1.0, 0.0, 0.0, 1.0 },
		type = "vec4",
		index = 1, -- u_UserData1
	},
}

kernel.fragment =
[[
uniform P_COLOR vec4 u_UserData0; // darkColor
uniform P_COLOR vec4 u_UserData1; // lightColor

const P_COLOR vec3 kWeights = vec3( 0.2125, 0.7154, 0.0721 );

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;
    P_COLOR float luminance = dot( texColor.rgb, kWeights );
    
	P_COLOR vec3 result = mix( u_UserData0.rgb, u_UserData1.rgb, luminance );

	// Pre-multiply alpha.
	result.rgb *= texColor.a;

	return vec4( result.rgb, texColor.a );
}
]]

return kernel
