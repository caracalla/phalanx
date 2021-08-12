#define _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <shaderc/shaderc.hpp>


// this was my testbench for using libshaderc to compile shaders within my
// program, instead of doing it beforehand


// file loading helper, for shaders
static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}



int main() {
	std::cout << "here we go\n";
	
	std::string shader_file_name = "shader.frag";

	std::vector<char> shader_glsl = readFile(shader_file_name);
	if (shader_glsl.empty()) {
		std::cout << "empty file!\n";
	}

	shaderc::CompileOptions compile_options;
	compile_options.SetSourceLanguage(shaderc_source_language_glsl);

	shaderc::Compiler compiler;
	const auto compilation_result = compiler.CompileGlslToSpv(
			shader_glsl.data(),
			shader_glsl.size(),
			shaderc_glsl_default_fragment_shader,
			shader_file_name.c_str(),
			"main",
			compile_options);

	if (compilation_result.GetCompilationStatus() == shaderc_compilation_status_success) {
		std::cout << "it worked!!!\n";

		std::string output_file_name = "test.spv";
		std::ofstream output_file_stream;
		output_file_stream.open(output_file_name, std::ios_base::binary);
		
		if (output_file_stream.fail()) {
			std::cerr << "failed to open file " << output_file_name << std::endl;
			return 1;
		}

		std::vector<char> compilation_output(
				reinterpret_cast<const char*>(compilation_result.cbegin()),
				reinterpret_cast<const char*>(compilation_result.cend()));

		output_file_stream.write(compilation_output.data(), compilation_output.size());
		output_file_stream.close();
	} else {
		std::cout << "it didn't work :(\n";

		std::cerr << compilation_result.GetErrorMessage() << std::endl;
	}

	std::cout << "we're done!!!\n";
	return 0;
}
