local kernel = {}

kernel.language = "glsl"

kernel.category = "composite"

kernel.name = "normalMapWith1DirLight"

kernel.uniformData =
{
	{
		name = "dirLightColor",
		default = { 1, 1, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 0, -- u_UserData0
	},
	{
		name = "dirLightDirection",
		default = { 1, 0, 0 },
		min = { 0, 0, 0 },
		max = { 1, 1, 1 },
		type="vec3",
		index = 1, -- u_UserData1
	},
	{
		name = "ambientLightIntensity",
		default = 0,
		min = 0,
		max = 1,
		type="scalar",
		index = 2, -- u_UserData2
	},
}

kernel.vertex =
[[
uniform P_COLOR vec4 u_UserData0; // dirLightColor
uniform P_NORMAL vec3 u_UserData1; // dirLightDirection
uniform P_COLOR float u_UserData2; // ambientLightIntensity

varying P_COLOR vec3 dirLightColor;
varying P_NORMAL vec3 dirLightDirection;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	// Pre-multiply the light color with its intensity.
	dirLightColor = ( u_UserData0.rgb * u_UserData0.a );

	dirLightDirection = normalize( u_UserData1 );

	return position;
}
]]

kernel.fragment =
[[
uniform P_COLOR vec4 u_UserData0; // dirLightColor
uniform P_NORMAL vec3 u_UserData1; // dirLightDirection
uniform P_COLOR float u_UserData2; // ambientLightIntensity

varying P_COLOR vec3 dirLightColor;
varying P_NORMAL vec3 dirLightDirection;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR float ambientLightIntensity = u_UserData2;

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// The base (diffuse) color.
    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	// The normal map.
    P_NORMAL vec3 surface_normal = texture2D( u_FillSampler1, texCoord ).xyz;

	// Transform from 0.0 .. 1.0 to -1.0 .. 1.0 range.
	surface_normal.xyz = normalize( ( surface_normal.xyz * 2.0 ) - 1.0 );

	// Apply light intensity.
	// We use "max()" to avoid negative intensities.
	P_NORMAL float diffuse_intensity = max( dot( dirLightDirection,
												surface_normal ),
										0.0 );

	// Add directional light color.
	texColor.rgb *= ( dirLightColor * ( diffuse_intensity + ambientLightIntensity ) );

	#if 0 // For debugging ONLY.

		// Return a solid color to represent the light.
		// This could be partially moved to the vertex shader.

		P_UV vec2 light_position_in_tc = ( ( dirLightDirection.xy + 1.0 ) * 0.5 );

		P_UV float light_distance = distance( texCoord, light_position_in_tc );

		// We DON'T want this to be proportional to u_TexelSize because
		// we want the circle to be of a constant size, NOT proportional
		// to the texture resolution.

		if( light_distance < ( 1.0 / 92.0 ) )
		{
			return vec4( 0.0, 0.0, 0.0, 1.0 );
		}
		else if( light_distance < ( 1.0 / 64.0 ) )
		{
			return vec4( 1.0, 1.0, 1.0, 1.0 );
		}

	#endif

	return ( texColor * v_ColorScale );
}
]]

return kernel
