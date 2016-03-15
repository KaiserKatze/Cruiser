TOOL=g++
DIR_BUILD=build
INCLUDE=-I.
MACRO=-DDEBUG -DLOG_ERROR -DLOG_INFO -DQUICK_REFERENCE
SOURCE_MAIN=`find -type f \( -name "*.c" -or -name "*.cpp" \) -and ! -name "test.c*"`
LIB_MAIN=`pkg-config --libs libzip` -lm
EXEC=cruise

cruise: main.c java.c java.h jar.c jar.h log.c log.h attr.c
	@clear
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	@echo "Tool   : "${TOOL}
	@echo "Macros : "${MACRO}
	@echo "Target : "${DIR_BUILD}/${EXEC}
	@echo "\r\n"
	@${TOOL} -g -o ${DIR_BUILD}/${EXEC} ${SOURCE_MAIN} ${INCLUDE} ${LIB_MAIN} ${MACRO}

test: test.c
	@clear
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	${TOOL} -g -o ${DIR_BUILD}/test test.c

clean:
	@clear
	@if [ -d ${DIR_BUILD} ]; then rm -r ${DIR_BUILD}; fi
	@find -type f -name "*.log" -delete
	@find -maxdepth 1 -type f -executable -delete
