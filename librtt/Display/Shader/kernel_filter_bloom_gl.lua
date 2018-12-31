local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "bloom"

kernel.graph = 
{
	nodes = {
		levels		=	{ effect="filter.levels",			input1="paint1"							},
		blur		=	{ effect="filter.blurGaussian",		input1="levels"							},
		--horizontal		=	{ effect="filter.blurHorizontal",		input1="levels"				},
		--vertical		=	{ effect="filter.blurVertical",		input1="horizontal"					},
		add			=	{ effect="composite.add",			input1="blur",		input2="paint1",	},
	},
	output = "add",
}

return kernel