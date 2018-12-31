local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "colorPolynomial"

kernel.uniformData =
{
	{
		name = "coefficients",
		default = { 1.0, 0.0, 0.0, 0.0,
					0.0, 1.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0, 0.0, 0.0, 1.0 },
		min = { 0.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.0 },
		max = { 0.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.0 },
		type="mat4",
		index = 0, -- u_UserData0
	},
}

kernel.fragment =
[[
uniform P_COLOR mat4 u_UserData0; // coefficients

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR mat4 coefficients = u_UserData0;

	P_COLOR vec4 input_color = texture2D( u_FillSampler0, texCoord );

	P_COLOR vec4 redCoefficients = coefficients[ 0 ];
	P_COLOR vec4 greenCoefficients = coefficients[ 1 ];
	P_COLOR vec4 blueCoefficients = coefficients[ 2 ];
	P_COLOR vec4 alphaCoefficients = coefficients[ 3 ];

	P_COLOR float r = ( redCoefficients.x +
					( redCoefficients.y * input_color.r ) +
					( redCoefficients.z * input_color.r * input_color.r ) +
					( redCoefficients.w * input_color.r * input_color.r * input_color.r ) );

	P_COLOR float g = ( greenCoefficients.x +
						( greenCoefficients.y * input_color.g ) +
						( greenCoefficients.z * input_color.g * input_color.g ) +
						( greenCoefficients.w * input_color.g * input_color.g * input_color.g ) );

	P_COLOR float b = ( blueCoefficients.x +
						( blueCoefficients.y * input_color.b ) +
						( blueCoefficients.z * input_color.b * input_color.b ) +
						( blueCoefficients.w * input_color.b * input_color.b * input_color.b ) );

	P_COLOR float a = ( alphaCoefficients.x +
						( alphaCoefficients.y * input_color.a ) +
						( alphaCoefficients.z * input_color.a * input_color.a ) +
						( alphaCoefficients.w * input_color.a * input_color.a * input_color.a ) );

	return ( vec4( r, g, b, a ) * v_ColorScale );
}
]]

return kernel
