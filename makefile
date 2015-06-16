cruise : main.c java.c java.h
	gcc -o build/cruise main.c java.c -I.

test: jar.h jar.c
	gcc -o build/test jar.c -I. `pkg-config --libs libzip`

clean:
	find build -type f -executable -delete
	find -type f -name "*.log" -delete
