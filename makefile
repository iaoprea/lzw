CC = gcc
CFLAGS = -std=c89 -pedantic -Wall -Wextra
TARGET = unlzw

default: build

debug: CFLAGS += -g -DDEBUG -lefence 
debug: build

build: $(TARGET)

run:
	./$(TARGET) compressedfile1 output1
	./$(TARGET) compressedfile2 output2
	./$(TARGET) compressedfile3 output3

run_debug:
	valgrind --tool=memcheck --leak-check=full --track-origins=yes \
		./$(TARGET) compressedfile2 output2

coding_style:
	#wget http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/plain/scripts/checkpatch.pl
	#chmod +x checkpatch.pl
	./checkpatch.pl --no-tree -f unlzw.c \
		--ignore LONG_LINE,LONG_LINE_COMMENT,AVOID_EXTERNS

cppcheck:
	#apt-get install cppcheck
	cppcheck -v --std=c89 --enable=all $(TARGET).c


clean:
	$(RM) $(TARGET) *.o *~
