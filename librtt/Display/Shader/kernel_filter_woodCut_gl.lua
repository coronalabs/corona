local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "woodCut"

kernel.vertexData =
{
	{
		name = "intensity",
		default = 0.5,
		min = 0,
		max = 1,
		index = 0, -- a_UserData.x
	},
}

kernel.vertex =
[[
varying P_COLOR float intensity_squared;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	P_COLOR float unitIntensity = a_UserData.x;

	intensity_squared = ( unitIntensity * unitIntensity );

	return position;
}
]]

kernel.fragment =
[[
varying P_COLOR float intensity_squared;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR vec2 o_x = vec2(u_TexelSize.x, 0.0);
	P_COLOR vec2 o_y = vec2(0.0, u_TexelSize.y);
	P_COLOR vec2 pp = texCoord - o_y;
	P_COLOR float t00 = texture2D(u_FillSampler0, pp - o_x).x;
	P_COLOR float t01 = texture2D(u_FillSampler0, pp).x;
	P_COLOR float t02 = texture2D(u_FillSampler0, pp + o_x).x;
	pp = texCoord;
	P_COLOR float t10 = texture2D(u_FillSampler0, pp - o_x).x;

	P_COLOR float t12 = texture2D(u_FillSampler0, pp + o_x).x;
	pp = texCoord + o_y;
	P_COLOR float t20 = texture2D(u_FillSampler0, pp - o_x).x;
	P_COLOR float t21 = texture2D(u_FillSampler0, pp).x;
	P_COLOR float t22 = texture2D(u_FillSampler0, pp + o_x).x;
	P_COLOR float s_x = t20 + t22 - t00 - t02 + 2.0 * (t21 - t01);
	P_COLOR float s_y = t22 + t02 - t00 - t20 + 2.0 * (t12 - t10);
	P_COLOR float dist = (s_x * s_x + s_y * s_y);

	// The result is mostly black, with white highlights.
	// step( a, b ) = ( ( a <= b ) ? 1.0 : 0.0 ).
	return ( vec4( step( intensity_squared, dist ) ) * v_ColorScale );
}
]]

return kernel
