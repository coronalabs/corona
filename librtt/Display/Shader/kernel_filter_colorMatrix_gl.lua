local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "colorMatrix"

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
	{
		name = "bias",
		default = { 0.0, 0.0, 0.0, 0.0 },
		min = { -1.0, -1.0, -1.0, -1.0 },
		max = { 1.0, 1.0, 1.0, 1.0 },
		type="vec4",
		index = 1, -- u_UserData1
	},
}

kernel.fragment =
[[
uniform P_COLOR mat4 u_UserData0; // coefficients
uniform P_COLOR vec4 u_UserData1; // bias

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_COLOR vec4 input_color = texture2D( u_FillSampler0, texCoord );

	P_COLOR mat4 coefficients = u_UserData0;
	P_COLOR vec4 bias = u_UserData1;

	P_COLOR vec4 redCoefficients = coefficients[ 0 ];
	P_COLOR vec4 greenCoefficients = coefficients[ 1 ];
	P_COLOR vec4 blueCoefficients = coefficients[ 2 ];
	P_COLOR vec4 alphaCoefficients = coefficients[ 3 ];

	P_COLOR float r = dot( input_color, redCoefficients );
	P_COLOR float g = dot( input_color, greenCoefficients );
	P_COLOR float b = dot( input_color, blueCoefficients );
	P_COLOR float a = dot( input_color, alphaCoefficients );

	return ( ( vec4( r, g, b, a ) + bias ) * v_ColorScale );
}
]]

return kernel
