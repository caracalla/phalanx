CFLAGS = -std=c++17 -O0
LDFLAGS = -lglfw -framework Cocoa -lvulkan

phalanx:
	clang++ $(CFLAGS) -o phalanx main.cpp $(LDFLAGS)

clean:
	rm -f phalanx

run: clean phalanx
	./phalanx
