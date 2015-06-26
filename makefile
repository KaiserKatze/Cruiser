SOURCE=`find -type f -name "*.c"`

cruise : main.c java.c java.h jar.c jar.h log.c log.h attr.c
	if [ ! -d build ]; then mkdir build; fi
	gcc -g -o build/cruise ${SOURCE} -I. `pkg-config --libs libzip` -lm -DDEBUG -DLOG_ERROR -DLOG_INFO

clean:
	if [ -d build ]; then rm -r build; fi
	find -type f -name "*.log" -delete
	find -maxdepth 1 -type f -executable -delete
