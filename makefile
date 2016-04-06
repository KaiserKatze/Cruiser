TOOL=g++
DIR_BUILD=build
INCLUDE=-I.
MACRO=-DDEBUG -DLOG_ERROR -DLOG_INFO
# -DQUICK_REFERENCE
SOURCE_MAIN=`find -type f \( -name "*.c" -or -name "*.cpp" \) -and ! -name "test.c*" -and ! -name "jar.c*"`
LIB_MAIN=`pkg-config --libs libzip` -lm
EXEC=cruise


LOG="Tool   : "${TOOL}"\r\nMacros : "${MACRO}"\r\nTarget : "${DIR_BUILD}/${EXEC}"\r\n"

cruise: main.c java.c java.h jar.c jar.h log.c log.h attr.c
	@clear
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	@echo ${LOG}
	@${TOOL} -g -o ${DIR_BUILD}/${EXEC} ${SOURCE_MAIN} ${INCLUDE} ${LIB_MAIN} ${MACRO}

rt: rt.h rt.cpp
	@clear
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	@echo ${LOG}
	@${TOOL} -g -c -o ${DIR_BUILD}/rt.o rt.cpp ${INCLUDE} ${MACRO}

test: test.c
	@clear
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	${TOOL} -g -o ${DIR_BUILD}/test test.c

clean:
	@clear
	@if [ -d ${DIR_BUILD} ]; then rm -r ${DIR_BUILD}; fi
	@find -type f -name "*.log" -delete
	@find -maxdepth 1 -type f -executable -delete
