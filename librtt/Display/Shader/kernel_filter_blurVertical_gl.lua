local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "blurVertical"

kernel.unsupportedPlatforms =
{
	WinPhone = true,
}

kernel.vertexData =
{
	{
		name = "blurSize",
		default = 4,
		min = 2,
		max = 128,
		index = 0, -- v_UserData.x
	},
	{
		name = "sigma",
		default = 128,
		min = 2,
		max = 512,
		index = 1, -- v_UserData.y
	},
}

kernel.fragment =
[[
const P_UV float kPI = 3.14159265359;

P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
	P_UV float blurSize = v_UserData.x;
	P_UV float sigma = v_UserData.y;

	P_UV float num_blur_pixels_per_side = ( floor( blurSize ) / 2.0 );

	// Direction.
	P_UV vec2 blur_multiply_dir = vec2(0.0, 1.0);

	P_UV vec3 incremental_gaussian;
	incremental_gaussian.x = 1.0 / (sqrt(2.0 * kPI) * sigma);
	incremental_gaussian.y = exp(-0.5 / (sigma * sigma));
	incremental_gaussian.z = incremental_gaussian.y * incremental_gaussian.y;

	P_UV vec4 avg_value = vec4(0.0, 0.0, 0.0, 0.0);
	P_UV float coefficient_sum = 0.0;

	// Center.
	avg_value += texture2D(u_FillSampler0, texCoord.st) * incremental_gaussian.x;
	coefficient_sum += incremental_gaussian.x;
	incremental_gaussian.xy *= incremental_gaussian.yz;

	// Sample on each side of the center.
	for( P_UV float i = 1.0;
			i <= 64.0; i++ ) // 64: This is half the blurSize.
	{ 
		if( i > num_blur_pixels_per_side )
		{
			break;
		}
		avg_value += texture2D(u_FillSampler0, texCoord.st - i * u_TexelSize.xy *
								blur_multiply_dir) * incremental_gaussian.x;
		avg_value += texture2D(u_FillSampler0, texCoord.st + i * u_TexelSize.xy *
								blur_multiply_dir) * incremental_gaussian.x;
		coefficient_sum += 2.0 * incremental_gaussian.x;
		incremental_gaussian.xy *= incremental_gaussian.yz;
	}

	return ( avg_value / coefficient_sum ) * v_ColorScale;
}
]]

return kernel
