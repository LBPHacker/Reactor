reactorIncdirs += [
	include_directories('.'),
]
reactorSources += files(
	'Movement.cpp',
	'Scene.cpp',
	'main.cpp',
)
foreach name : [
	'Vert',
	'Frag',
	'Rintent',
	'Rapply',
]
	reactorSources += toArray.process('@0@.glsl'.format(name), extra_args: name)
endforeach

subdir('Common')
subdir('Gl')
