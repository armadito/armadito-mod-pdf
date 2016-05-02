all:
	gcc -g -o uhPDFAnalyzer -Wall -Wextra main.c src/*.c -Iincludes/

parser:
	gcc -o uhPDFParser -Wall -O3 -Wextra parser.c src/*.c -Iincludes/
	#-d fortify sources

lib:
	gcc -fPIC -g -c -Wall -Wextra src/*.c -Iincludes
	gcc -shared -Wl,-soname,uhPDFAnalyzer.so.1 -o uhPDFAnalyzer-1.0.1.so *.o -lc
	

clean:
	rm uhPDFAnalyzer
	rm *.o



