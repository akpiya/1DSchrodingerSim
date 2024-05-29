.PHONY: clean

test: src/solver.c tests/test.c
	clang -Wall src/solver.c tests/test.c -o test -lm -lcriterion

clean:
	rm -f quantum test

quantum: src/solver.c src/quantumapp.c src/libraylib.a
	clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL src/libraylib.a src/quantumapp.c src/solver.c -o quantum