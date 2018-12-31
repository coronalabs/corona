local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "wobble"

kernel.isTimeDependent = true

kernel.vertexData =
{
	{
		name = "amplitude",
		default = 10,
		-- min ?
		-- max ?
		index = 0, -- a_UserData.x
	},
}

kernel.vertex =
[[
P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
    position.y += sin( 3.0 * u_TotalTime + a_TexCoord.x ) * a_UserData.x * a_TexCoord.y;

	return position;
}
]]

return kernel
