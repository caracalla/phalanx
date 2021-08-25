#pragma once


#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "texture.h"
#include "vertex.h"


struct Model {
	std::vector<Vertex> vertices;
	// std::vector<uint16_t> indices;
	std::vector<uint32_t> indices;
	Texture* texture;

	static Model load(const char* filename) {
		Model model;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string error;

		if (
				!tinyobj::LoadObj(
						&attrib, &shapes, &materials, &warn, &error, filename)) {
			throw std::runtime_error(warn + error);
		}

		for (const auto& shape: shapes) {
			for (const auto& index: shape.mesh.indices) {
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // OBJ format has 0 at the bottom of the image, we do the opposite
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				model.vertices.push_back(vertex);

				// not de-duping vertices yet, so indices just increment
				model.indices.push_back(model.indices.size());
			}
		}

		return model;
	}
};
