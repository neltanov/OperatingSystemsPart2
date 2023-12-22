TARGET_1 = spinlock_list
SRCS_1 = spinlock_main.c spinlock_list.c 

TARGET_2 = mutex_list
SRCS_2 = mutex_main.c mutex_list.c 

TARGET_3 = rwlock_list
SRCS_3 = rwlock_main.c rwlock_list.c 

CC=gcc
RM=rm
CFLAGS= -g -Wall
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1} ${TARGET_2} ${TARGET_3}

${TARGET_1}: spinlock_list.h utils.h ${SRCS_1}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_1} ${LIBS} -o ${TARGET_1}
${TARGET_2}: mutex_list.h utils.h ${SRCS_2}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_2} ${LIBS} -o ${TARGET_2}
${TARGET_3}: rwlock_list.h utils.h ${SRCS_3}
	${CC} ${CFLAGS} -I${INCLUDE_DIR} ${SRCS_3} ${LIBS} -o ${TARGET_3}

clean:
	${RM} -f *.o ${TARGET_1} ${TARGET_2} ${TARGET_3}