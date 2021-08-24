CFLAGS = -std=c++17 -O0
LDFLAGS = -lglfw -framework Cocoa -lvulkan

.PHONY: clean shaders run

phalanx:
	clang++ $(CFLAGS) -o phalanx main.cpp $(LDFLAGS)

clean:
	rm -f phalanx
	# rm -f shaders/frag.spv shaders/vert.spv

shaders:
	glslc shaders/shader.frag -o shaders/frag.spv
	glslc shaders/shader.vert -o shaders/vert.spv

run: clean phalanx shaders
	./phalanx
