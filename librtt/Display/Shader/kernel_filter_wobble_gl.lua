local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "wobble"

--kernel.isTimeDependent = true
kernel.timeTransform = {
	func = "modulo",
	range = ( 2 / 3 ) * math.pi -- cf. VertexKernel()
}

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
	// sine is periodic, so we can use a short modular range
	// at t = 0, sin( 3 * 0 ) = 0 (ignoring phase)
	// this will next happen as sin( 2 * pi ) = 0, i.e. when 3 * t = 2 * pi
	// rearranging, t = ( 2 / 3 ) * pi

    position.y += sin( 3.0 * u_TotalTime + a_TexCoord.x ) * a_UserData.x * a_TexCoord.y;

	return position;
}
]]

return kernel
