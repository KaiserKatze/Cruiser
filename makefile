cruise : main.c java.c java.h jar.c jar.h
	if [ ! -d build ]; then mkdir build; fi
	gcc -g -o build/cruise main.c java.c jar.c -I. `pkg-config --libs libzip`

clean:
	if [ -d build ]; then rm -r build; fi
	find -type f -name "*.log" -delete
