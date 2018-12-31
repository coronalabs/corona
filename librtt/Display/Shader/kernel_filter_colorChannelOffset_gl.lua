local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "colorChannelOffset"

kernel.vertexData =
{
	{
		name = "xTexels",
		default = 8,
		min = 0,
		max = 99999,
		index = 0, -- v_UserData.x
	},
	{
		name = "yTexels",
		default = 8,
		min = 0,
		max = 99999,
		index = 1, -- v_UserData.y
	},
}

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	// "u_TexelSize.xy" is in pixels.
	// "u_TexelSize.zw" is in content units.
	P_POSITION vec2 channelOffset = ( v_UserData.xy * u_TexelSize.zw );

	// Red channel.
	P_COLOR vec4 r = texture2D( u_FillSampler0,
								( texCoord - channelOffset ) );

	// Keep the green channel where it is.
	P_COLOR vec4 g = texture2D( u_FillSampler0,
								texCoord );

	// Blue channel.
	P_COLOR vec4 b = texture2D( u_FillSampler0,
								( texCoord + channelOffset ) );

	// Combine.
	return ( vec4( r.r, g.g, b.b, g.a ) * v_ColorScale );
}
]]

return kernel
