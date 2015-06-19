SOURCE=`find -type f -name "*.c"`

cruise : main.c java.c java.h jar.c jar.h log.c log.h
	if [ ! -d build ]; then mkdir build; fi
	gcc -g -o build/cruise ${SOURCE} -I. `pkg-config --libs libzip` -DDEBUG -DLOG_ERROR

clean:
	if [ -d build ]; then rm -r build; fi
	find -type f -name "*.log" -delete
