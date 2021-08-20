CFLAGS = -std=c++17 -O0
LDFLAGS = -lglfw -framework Cocoa -lvulkan

phalanx:
	clang++ $(CFLAGS) -o phalanx main.cpp $(LDFLAGS)

clean:
	rm -f phalanx

shaders:
	glslc shader.frag -o frag.spv
	glslc shader.vert -o vert.spv

run: clean phalanx shaders
	./phalanx
