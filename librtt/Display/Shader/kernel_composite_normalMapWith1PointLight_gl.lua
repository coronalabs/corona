local kernel = {}

kernel.language = "glsl"

kernel.category = "composite"

kernel.name = "normalMapWith1PointLight"

kernel.uniformData =
{
	{
		name = "pointLightColor",
		default = { 1, 1, 1, 1 },
		min = { 0, 0, 0, 0 },
		max = { 1, 1, 1, 1 },
		type="vec4",
		index = 0, -- u_UserData0
	},
	{
		name = "pointLightPos",
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
	{
		name = "attenuationFactors",
		default = { 0.4, 3, 20 },
		min = { 0, 0, 0 },
		max = { 99999, 99999, 99999 },
		type="vec3",
		index = 3, -- u_UserData3
	},
}

kernel.vertex =
[[
uniform P_COLOR vec4 u_UserData0; // pointLightColor
uniform P_UV vec3 u_UserData1; // pointLightPos
uniform P_COLOR float u_UserData2; // ambientLightIntensity
uniform P_COLOR vec3 u_UserData3; // attenuationFactors

varying P_COLOR vec3 pointLightColor;

P_POSITION vec2 VertexKernel( P_POSITION vec2 position )
{
	// Pre-multiply the light color with its intensity.
	pointLightColor = ( u_UserData0.rgb * u_UserData0.a );

	//Use global position instead of positioning on UVs
	//v_Position = position;
	return position;
}
]]

kernel.fragment =
[[
uniform P_COLOR vec4 u_UserData0; // pointLightColor
uniform P_UV vec3 u_UserData1; // pointLightPos
uniform P_COLOR float u_UserData2; // ambientLightIntensity
uniform P_COLOR vec3 u_UserData3; // attenuationFactors

varying P_COLOR vec3 pointLightColor;

P_UV float GetDistanceAttenuation( in P_UV vec3 attenuationFactors,
									in P_UV float light_distance )
{
	P_UV float constant_attenuation_factor = attenuationFactors.x;
	P_UV float linear_attenuation_factor = attenuationFactors.y;
	P_UV float quadratic_attenuation_factor = attenuationFactors.z;

	P_UV float constant_attenuation = constant_attenuation_factor;
	P_UV float linear_attenuation = ( linear_attenuation_factor * light_distance );
	P_UV float quadratic_attenuation = ( quadratic_attenuation_factor * light_distance * light_distance );

	return ( 1.0 / ( constant_attenuation +
						linear_attenuation +
						quadratic_attenuation ) );
}

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV vec3 pointLightPos = u_UserData1;
	P_UV float ambientLightIntensity = u_UserData2;

	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////

	// The base (diffuse) color.
    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord );

	// The normal map.
    P_NORMAL vec3 surface_normal = texture2D( u_FillSampler1, texCoord ).xyz;

	// Transform from 0.0 .. 1.0 to -1.0 .. 1.0 range.
	surface_normal.xyz = normalize( ( surface_normal.xyz * 2.0 ) - 1.0 );

	//! \TODO Add an option to support "Y+ up" and "Y+ down"?
	//! Instead of providing an option used in the shader,
	//! we can provide two different shaders that assume
	//! different Y+ directions.
	//surface_normal.y = ( - surface_normal.y );

	P_UV vec3 fragment_to_light = ( pointLightPos - vec3( texCoord, 0.0 ) );
	//Use global position instead of positioning on UVs
	//P_UV vec3 fragment_to_light = ( pointLightPos - vec3( v_Position*CoronaTexelSize.xy, 0.0 ) );

	P_UV vec3 light_direction = normalize( fragment_to_light );

	// Distance attenuation.
	P_UV float attenuation = GetDistanceAttenuation( u_UserData3,
														length( fragment_to_light ) );

	// Apply light intensity.
	// We use "max()" to avoid negative intensities.
	P_UV float diffuse_intensity = max( dot( light_direction,
												surface_normal ),
										0.0 );

	// Apply light distance attenuation.
	diffuse_intensity *= attenuation;

	// Add point light color.
	texColor.rgb *= ( pointLightColor * ( diffuse_intensity + ambientLightIntensity ) );

	#if 0 // For debugging ONLY.

		// Return a solid color to represent the light.
		// This could be partially moved to the vertex shader.

		P_UV float light_distance = distance( texCoord, pointLightPos.xy );

		// We DON'T want this to be proportional to u_TexelSize because
		// we want the circle to be of a constant size, NOT proportional
		// to the texture resolution.
		const P_UV float inner_threshold = ( 1.0 / 92.0 );
		const P_UV float outer_threshold = ( 1.0 / 64.0 );

		if( light_distance < inner_threshold )
		{
			if( pointLightPos.z >= 0.0 )
			{
				// Green when the light is in front of the surface.
				return vec4( 0.0, 1.0, 0.0, 1.0 );
			}
			else
			{
				// Red when the light is behind of the surface.
				return vec4( 1.0, 0.0, 0.0, 1.0 );
			}
		}
		else if( light_distance < outer_threshold )
		{
			// The outline is always white.
			return vec4( 1.0, 1.0, 1.0, 1.0 );
		}

	#endif

	#if 0 // For debugging ONLY.

		return vec4( attenuation );

	#endif

	return ( texColor * v_ColorScale );
}
]]

return kernel
