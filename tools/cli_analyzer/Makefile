all:
	gcc -g -o a6oPDFAnalyzer -Wall -Wextra main.c ../../lib/src/*.c -I../../lib/includes

lib:
	gcc -fPIC -g -c -Wall -Wextra src/*.c -Iincludes
	gcc -shared -Wl,-soname,a6oPDFAnalyzer.so.1 -o a6oPDFAnalyzer-1.0.1.so *.o -lc
	

clean:
	rm a6oPDFAnalyzer
	rm *.o
