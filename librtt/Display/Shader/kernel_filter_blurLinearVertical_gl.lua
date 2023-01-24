local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "blurLinearVertical"

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

kernel.vertexData =
{
	{
		name = "offset1",
		default = 1.3846153846,
		min = 0,
		max = 100,
		index = 0, -- v_UserData.x
	},
	{
		name = "offset2",
		default = 3.2307692308,
		min = 0,
		max = 100,
		index = 1, -- v_UserData.y
	},
}

kernel.fragment =
[[
const P_UV float kWeight0 = 0.2270270270;
const P_UV float kWeight1 = 0.3162162162;
const P_UV float kWeight2 = 0.0702702703;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
  P_COLOR vec4 color = texture2D(u_FillSampler0, texCoord.st) * kWeight0;

  color += texture2D(u_FillSampler0, (texCoord.st + vec2(0.0, v_UserData.x) * u_TexelSize.xy)) * kWeight1;
  color += texture2D(u_FillSampler0, (texCoord.st - vec2(0.0, v_UserData.x) * u_TexelSize.xy)) * kWeight1;
  color += texture2D(u_FillSampler0, (texCoord.st + vec2(0.0, v_UserData.y) * u_TexelSize.xy)) * kWeight2;
  color += texture2D(u_FillSampler0, (texCoord.st - vec2(0.0, v_UserData.y) * u_TexelSize.xy)) * kWeight2;

  return color * v_ColorScale;
}
]]

return kernel
