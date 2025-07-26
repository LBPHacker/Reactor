import os
import re
import sys

(
	script,
	output_cpp_path,
	output_hpp_path,
	output_dep_path,
	input_path,
	symbol_name,
) = sys.argv

script_path = os.path.realpath(__file__)

with open(input_path, 'rb') as input_f:
	data = input_f.read()
data_size = len(data)
bytes_str = ', '.join([ str(ch) for ch in data ])

with open(output_cpp_path, 'w') as output_cpp_f:
	output_cpp_f.write(f'''
#include "{output_hpp_path}"

namespace Reactor::Resource
{{
	const struct {symbol_name}Resource {symbol_name} = {{{{{{ {bytes_str} }}}}}};
}}
''')

with open(output_hpp_path, 'w') as output_hpp_f:
	output_hpp_f.write(f'''
#pragma once
#include <array>
#include <span>

namespace Reactor::Resource
{{
	extern const struct {symbol_name}Resource
	{{
		std::array<unsigned char, {data_size}> data;

		std::span<const char> AsCharSpan() const
		{{
			return std::span(reinterpret_cast<const char *>(data.data()), data.size());
		}}

		std::span<const unsigned char> AsUcharSpan() const
		{{
			return std::span(data.data(), data.size());
		}}
	}} {symbol_name};
}}
''')

def dep_escape(s):
	return re.sub(r'[\\$ :]', '\\\1', s)

with open(output_dep_path, 'w') as output_dep_f:
	output_dep_f.write(f'''
{dep_escape(output_cpp_path)} {dep_escape(output_hpp_path)}: {dep_escape(input_path)} {dep_escape(script_path)}
''')
