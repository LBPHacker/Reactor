project('reactor', 'cpp', meson_version: '>=1.1.0')

projectOptions = [
	'cpp_std=c++20',
	'cpp_rtti=false',
	'warning_level=3',
]

python = import('python')
python3Prog = python.find_installation('python3')
toArray = generator(
	python3Prog,
	output: [ '@PLAINNAME@.cpp', '@PLAINNAME@.hpp' ],
	depfile: '@PLAINNAME@.dep',
	arguments: [
		join_paths(meson.current_source_dir(), 'ToArray.py'),
		'@OUTPUT0@',
		'@OUTPUT1@',
		'@DEPFILE@',
		'@INPUT@',
		'@EXTRA_ARGS@',
	]
)

reactorIncdirs = []
reactorSources = []

subdir('Reactor')

executable(
	'reactor',
	sources: reactorSources,
	include_directories: reactorIncdirs,
	dependencies: [
		dependency('sdl2'),
		dependency('glm'),
	],
	override_options: projectOptions,
)
