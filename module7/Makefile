all:
	gcc -g -o uhPDFAnalyzer -Wall -Wextra main.c src/*.c -Iincludes/

lib:
	gcc -fPIC -g -c -Wall -Wextra src/*.c -Iincludes
	gcc -shared -Wl,-soname,uhPDFAnalyzer.so.1 -o uhPDFAnalyzer-1.0.1.so *.o -lc
	

clean:
	rm uhPDFAnalyzer
	rm *.o



