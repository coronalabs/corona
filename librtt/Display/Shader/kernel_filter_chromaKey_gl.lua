local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "chromaKey"

kernel.uniformData =
{
	{
		name = "sensitivity",
		default = 0.4,
		min = 0,
		max = 1,
		type="scalar",
		index = 0, -- u_UserData0
	},
	{
		name = "smoothing",
		default = 0.1,
		min = 0,
		max = 1,
		type="scalar",
		index = 1, -- u_UserData1
	},
	{
		name = "color",
		default = { 1, 1, 1, 1 }, -- ??
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 2, -- u_UserData2
	},
}

kernel.fragment =
[[
uniform P_COLOR float u_UserData0; // sensitivity
uniform P_COLOR float u_UserData1; // smoothing
uniform P_COLOR vec4 u_UserData2; // color

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR float sensitivity = u_UserData0; // threshold
    P_COLOR float smoothing = u_UserData1;
    P_COLOR vec4 color = u_UserData2;

    P_COLOR vec4 texColor = ( texture2D( u_FillSampler0, texCoord ) * v_ColorScale );
    //P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

    P_COLOR float maskY = 0.2989 * color.r + 0.5866 * color.g + 0.1145 * color.b;
    P_COLOR float maskCr = 0.7132 * (color.r - maskY);
    P_COLOR float maskCb = 0.5647 * (color.b - maskY);
    
    P_COLOR float Y = 0.2989 * texColor.r + 0.5866 * texColor.g + 0.1145 * texColor.b;
    P_COLOR float Cr = 0.7132 * (texColor.r - Y);
    P_COLOR float Cb = 0.5647 * (texColor.b - Y);

    // P_COLOR float blendValue = 1.0 - smoothstep(sensitivity - smoothing, sensitivity , abs(Cr - maskCr) + abs(Cb - maskCb));
    P_COLOR float blendValue = smoothstep(sensitivity, sensitivity + smoothing, distance(vec2(Cr, Cb), vec2(maskCr, maskCb)));

	//return vec4(texColor.rgb, texColor.a * blendValue);
    return texColor * blendValue;
}
]]

return kernel
