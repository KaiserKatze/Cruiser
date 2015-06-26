SOURCE_MAIN=`find -type f -name "*.c" -and ! -name "test.c"`
SOURCE_TEST=`find -type f -name "*.c" -and ! -name "main.c"`

cruise : main.c java.c java.h jar.c jar.h log.c log.h attr.c
	if [ ! -d build ]; then mkdir build; fi
	gcc -g -o build/cruise ${SOURCE_MAIN} -I. `pkg-config --libs libzip` -lm -DDEBUG -DLOG_ERROR -DLOG_INFO

test: test.c
	if [ ! -d build ]; then mkdir build; fi
	gcc -g -o build/test ${SOURCE_TEST} -I. `pkg-config --libs libzip` -lm -DDEBUG -DLOG_ERROR -DLOG_INFO

clean:
	if [ -d build ]; then rm -r build; fi
	find -type f -name "*.log" -delete
	find -maxdepth 1 -type f -executable -delete
