.PHONY: clean

build: src/quantumapp.c src/solver.c src/potential.c
	clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -Iinclude/ -L lib/ -lraylib src/quantumapp.c src/solver.c lib/hashmap.c src/potential.c -o bin/quantum

run: build
	bin/quantum

scratch:
	clang -g -O0 -Wall src/solver.c src/hashmap.c tests/scratch.c -o bin/scratch -lm

test:
	clang -Wall src/solver.c tests/test.c src/hashmap.c -o bin/test -lm -lcriterion

clean:
	rm -f quantum test
