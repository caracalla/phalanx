#pragma once

#if PHALANX_DYNAMIC_SHADER_COMPILATION == 1
#include <shaderc/shaderc.hpp>
#endif // PHALANX_DYNAMIC_SHADER_COMPILATION == 1

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// file loading helper
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

#if PHALANX_DYNAMIC_SHADER_COMPILATION == 1
// we compile shaders on the fly, before rendering begins
std::vector<char> loadShader(
	const std::string& shaderFileName,
	shaderc_shader_kind shaderKind) {
	std::vector<char> shaderGlsl = readFile(shaderFileName);

	if (shaderGlsl.empty()) {
		throw std::runtime_error("empty shader file provided");
	}

	shaderc::CompileOptions compileOptions;
	compileOptions.SetSourceLanguage(shaderc_source_language_glsl);

	const char* entryPointName = "main";

	shaderc::Compiler compiler;
	const auto compilationResult = compiler.CompileGlslToSpv(
		shaderGlsl.data(),
		shaderGlsl.size(),
		shaderKind,
		shaderFileName.c_str(),
		entryPointName,
		compileOptions);

	if (
		compilationResult.GetCompilationStatus() !=
		shaderc_compilation_status_success) {
		std::cerr << "shader compilation error:\n";
		std::cerr << compilationResult.GetErrorMessage() << std::endl;

		throw std::runtime_error("failed to compile shader");
	}

	std::vector<char> compilationOutput(
		reinterpret_cast<const char*>(compilationResult.cbegin()),
		reinterpret_cast<const char*>(compilationResult.cend()));

	return compilationOutput;
}

std::vector<char> loadVertexShader(const std::string& shaderFileName) {
	return loadShader(shaderFileName, shaderc_glsl_default_vertex_shader);
}

std::vector<char> loadFragmentShader(const std::string& shaderFileName) {
	return loadShader(shaderFileName, shaderc_glsl_default_fragment_shader);
}
#endif // PHALANX_DYNAMIC_SHADER_COMPILATION == 1
