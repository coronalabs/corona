local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "invert"

kernel.fragment =
[[
P_COLOR vec4 FragmentKernel( P_UV vec2 texCoord )
{
    P_COLOR vec4 texColor = texture2D( u_FillSampler0, texCoord ) * v_ColorScale;

	// "texColor" has premultiplied alphas:
	//
	// texColor: ( ( r * a ), ( g * a ), ( b * a ), a )
	//
	// We only want to invert the RGB values, NOT the alpha.
	//
	// Therefore, we need to:
	//
	//		(1) Undo the effect of alpha on RGB.
	//		(2) Invert.
	//		(3) Reapply the alpha.
	//
	// In other words:
	//
	//		rgb_without_alpha = ( rgb / alpha );
	//		inverted_rgb_without_alpha = ( 1.0 - rgb_without_alpha );
	//		final_rgb_with_premultiplied_alpha = ( inverted_rgb_without_alpha * alpha );
	//
	//			Or:
	//
	//				vec3 result_rgb = ( ( 1.0 - ( rgb / a ) ) * a );
	//				vec3 result_rgb = ( a - rgb );
	//				vec4 result_rgba = ( ( a - rgb ), a );

    return vec4( ( texColor.a - texColor.rgb ), texColor.a );
}
]]

return kernel
