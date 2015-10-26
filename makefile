DIR_BUILD=build
INCLUDE=-I.
MACRO=-DDEBUG -DLOG_ERROR -DLOG_INFO
SOURCE_MAIN=`find -type f -name "*.c" -and ! -name "test.c"`
LIB_MAIN=`pkg-config --libs libzip` -lm
SOURCE_TEST=`find -type f -name "*.c" -and ! -name "main.c"`
LIB_TEST=`pkg-config --libs libzip` -lm

cruise: main.c java.c java.h jar.c jar.h log.c log.h attr.c
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	gcc -g -o ${DIR_BUILD}/cruise ${SOURCE_MAIN} ${INCLUDE} ${LIB_MAIN} ${MACRO} -Werror

test: test.c
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	gcc -g -o ${DIR_BUILD}/test ${SOURCE_TEST} ${INCLUDE} ${LIB_TEST} ${MACRO}

clean:
	@if [ -d ${DIR_BUILD} ]; then rm -r ${DIR_BUILD}; fi
	@find -type f -name "*.log" -delete
	@find -maxdepth 1 -type f -executable -delete
