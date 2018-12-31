local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "zoomBlur"

kernel.vertexData =
{
	{
		name = "u",
		default = 0.5,
		min = 0,
		max = 1,
		index = 0, -- v_UserData.x
	},
	{
		name = "v",
		default = 0.5,
		min = 0,
		max = 1,
		index = 1, -- v_UserData.y
	},
	{
		name = "intensity",
		default = 0.5,
		min = 0,
		max = 1,
		index = 2, -- v_UserData.z
	},
}

-- This is the same as a "radial blur".
kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV vec2 origin = vec2( v_UserData.x, v_UserData.y );
	P_UV float unitIntensity = v_UserData.z;

	P_UV vec2 samplingOffset = ( u_TexelSize.xy * (origin - texCoord) * ( 32.0 * unitIntensity ) );

	P_COLOR vec4 fragmentColor = texture2D(u_FillSampler0, texCoord) * 0.18;
	fragmentColor += texture2D(u_FillSampler0, texCoord + samplingOffset) * 0.15;
	fragmentColor += texture2D(u_FillSampler0, texCoord + (2.0 * samplingOffset)) *  0.12;
	fragmentColor += texture2D(u_FillSampler0, texCoord + (3.0 * samplingOffset)) * 0.09;
	fragmentColor += texture2D(u_FillSampler0, texCoord + (4.0 * samplingOffset)) * 0.05;
	fragmentColor += texture2D(u_FillSampler0, texCoord - samplingOffset) * 0.15;
	fragmentColor += texture2D(u_FillSampler0, texCoord - (2.0 * samplingOffset)) *  0.12;
	fragmentColor += texture2D(u_FillSampler0, texCoord - (3.0 * samplingOffset)) * 0.09;
	fragmentColor += texture2D(u_FillSampler0, texCoord - (4.0 * samplingOffset)) * 0.05;

	//return ( texture2D( u_FillSampler0, texCoord ) * v_ColorScale );
	return fragmentColor;
}
]]

return kernel
