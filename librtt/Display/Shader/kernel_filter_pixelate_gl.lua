local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "pixelate"

kernel.vertexData =
{
	{
		name = "numPixels",
		default = 4,
		min = 4,
		max = 99999,
		index = 0, -- a_UserData.x
	},
}

kernel.vertex =
[[
varying P_UV vec2 slot_size;
varying P_UV vec2 sample_uv_offset;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_UV float numPixels = a_UserData.x;

	// "u_TexelSize.xy" is in pixels.
	// "u_TexelSize.zw" is in content units.
	slot_size = ( u_TexelSize.zw * numPixels );

	// This is used to sample from the middle of the slot.
	sample_uv_offset = ( slot_size * 0.5 );

	return position;
}
]]

kernel.fragment =
[[
varying P_UV vec2 slot_size;
varying P_UV vec2 sample_uv_offset;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV vec2 uv = ( sample_uv_offset + ( floor( texCoord / slot_size ) * slot_size ) );

	return ( texture2D( u_FillSampler0, uv ) * v_ColorScale );
}
]]

return kernel
