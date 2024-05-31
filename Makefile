.PHONY: clean

scratch:
	clang -g -O0 -Wall src/solver.c src/hashmap.c tests/scratch.c -o bin/scratch -lm 

test:
	clang -Wall src/solver.c tests/test.c src/hashmap.c -o bin/test -lm -lcriterion

clean:
	rm -f quantum test

build:
	clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL src/libraylib.a src/quantumapp.c src/solver.c src/hashmap.c -o bin/quantum

run: build
	bin/quantum