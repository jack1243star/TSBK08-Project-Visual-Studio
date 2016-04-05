CFLAGS	= -lm -g -O0 -Wall -Wextra -pedantic
ENTROPY_SRC_DIR	= entropy/
ENTROPY_SRC	= $(ENTROPY_SRC_DIR)main.c \
                  $(ENTROPY_SRC_DIR)map.c \
                  $(ENTROPY_SRC_DIR)statistics.c
ENTROPY_EXE	= entropy.out
CODING_SRC_DIR	= source_coding/
CODING_SRC	= $(CODING_SRC_DIR)main.c \
                  $(CODING_SRC_DIR)bwt.c \
                  $(CODING_SRC_DIR)test_bwt.c
CODING_EXE	= source_coding.out

all	: $(ENTROPY_EXE) $(CODING_EXE)

$(ENTROPY_EXE)	: $(ENTROPY_SRC)
	$(CC) $(CFLAGS) -o $(ENTROPY_EXE) $(ENTROPY_SRC)

$(CODING_EXE)	: $(CODING_SRC)
	$(CC) $(CFLAGS) -o $(CODING_EXE) $(CODING_SRC)

clean	:
	-rm $(ENTROPY_EXE) $(CODING_EXE)
