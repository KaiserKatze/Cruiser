TOOL=g++
DIR_BUILD=build
INCLUDE=-I./include
MACRO=-DDEBUG -DLOG_ERROR -DLOG_INFO
LIB_MAIN=`pkg-config --libs libzip` -lm
EXEC=cruise


LOG="Tool   : "${TOOL}"\r\nMacros : "${MACRO}"\r\nTarget : "${DIR_BUILD}/${EXEC}"\r\n"

cruise: main.c java.c include/java.h attr.c
	@make init
	@clear;clear
	@echo ${LOG}
	@${TOOL} -g -o ${DIR_BUILD}/${EXEC} 				\
		main.c											\
		java.c											\
		attr.c											\
		${DIR_BUILD}/input.so							\
		${DIR_BUILD}/log.so								\
		${DIR_BUILD}/mem.so								\
		${DIR_BUILD}/vrf.so								\
		${DIR_BUILD}/rt.so								\
		${INCLUDE} ${LIB_MAIN} ${MACRO};

init:
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	@make input
	@make log
	@make mem
	@make vrf
	@make rt

# Modules
input: include/input.h input.c
	@${TOOL} -shared -o ${DIR_BUILD}/input.so 			\
		input.c 										\
		${INCLUDE} ${MACRO} ${LIB_MAIN}

log: include/log.h log.c
	@${TOOL} -shared -o ${DIR_BUILD}/log.so log.c 		\
		${INCLUDE} ${MACRO}

mem:
	@${TOOL} -shared -o ${DIR_BUILD}/mem.so memory.c 	\
		${INCLUDE} ${MACRO}

rt: include/rt.h rt.cpp
	@${TOOL} -g -shared -o ${DIR_BUILD}/rt.so rt.cpp 	\
		${INCLUDE} ${MACRO}

vrf: include/vrf.h vrf.c
	@${TOOL} -g -shared -o ${DIR_BUILD}/vrf.so vrf.c 	\
		${INCLUDE} ${MACRO}

# Test
test: test.c
	@clear
	@if [ ! -d ${DIR_BUILD} ]; then mkdir ${DIR_BUILD}; fi
	${TOOL} -g -o ${DIR_BUILD}/test test.c

# Clean
clean:
	@clear;clear
	@find ${DIR_BUILD} -type f -executable -delete
