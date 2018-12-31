local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "levels"

kernel.vertexData =
{
	{
		name = "white",
		default = 215/255,
		min = 0,
		max = 255,
		index = 0, -- v_UserData.x
	},
	{
		name = "black",
		default = 144/255,
		min = 0,
		max = 255,
		index = 1, -- v_UserData.y
	},
	{
		name = "gamma",
		default = 1.06,
		min = 0,
		max = 1,
		index = 2, -- v_UserData.z
	},
}

kernel.fragment =
[[
#define GammaCorrection(color, gamma)								pow(color, vec3(1.0 / gamma))

#define LevelsControlInputRange(color, minInput, maxInput)				min(max(color - vec3(minInput), vec3(0.0)) / (vec3(maxInput) - vec3(minInput)), vec3(1.0))
#define LevelsControlInput(color, minInput, gamma, maxInput)				GammaCorrection(LevelsControlInputRange(color, minInput, maxInput), gamma)
#define LevelsControlOutputRange(color, minOutput, maxOutput) 			mix(vec3(minOutput), vec3(maxOutput), color)
#define LevelsControl(color, minInput, gamma, maxInput, minOutput, maxOutput) 	LevelsControlOutputRange(LevelsControlInput(color, minInput, gamma, maxInput), minOutput, maxOutput)

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR float white = v_UserData.x;
	P_COLOR float black = v_UserData.y;
	P_COLOR float gamma = v_UserData.z;

	P_COLOR vec4 color = texture2D(u_FillSampler0, texCoord);
	return vec4( LevelsControl( color.rgb, black, gamma, white, 0.0, 1.0 ), color.a );
//	P_COLOR vec4 inPixel = texture2D(u_FillSampler0, texCoord);
//	P_COLOR vec4 color;
//	color.r = (pow(( (inPixel.r * 255.0) - black) / (white - black), gamma) ) / 255.0;
//	color.g = (pow(( (inPixel.g * 255.0) - black) / (white - black), gamma) ) / 255.0;
//	color.b = (pow(( (inPixel.b * 255.0) - black) / (white - black), gamma) ) / 255.0;
//	color.a = 1;
//	return color;
}
]]

return kernel
