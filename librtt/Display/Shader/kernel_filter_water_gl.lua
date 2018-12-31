local kernel = {}

kernel.language = "glsl"

kernel.category = "filter"

kernel.name = "water"

kernel.graph = 
{
	nodes = {
		blur		=	{ effect="filter.blurGaussian",	input1="paint1"	},
		step		=	{ effect="filter.step",			input1="blur"	},
	},
	output = "step",
}

return kernel
