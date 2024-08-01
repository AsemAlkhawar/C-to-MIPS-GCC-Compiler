CC = gcc
CFLAGS = -Wall -g


compile: parser.o driver.o scanner.o semantic.o ast-helper.o ast-print.o mips_gen.o gen_code.o
	$(CC) $(CFLAGS) -o compile parser.o driver.o scanner.o semantic.o ast-helper.o ast-print.o mips_gen.o gen_code.o

parser.o: parser.c scanner.h ast.h gen_code.h
	$(CC) $(CFLAGS) -c parser.c

driver.o: driver.c scanner.h ast.h gen_code.h
	$(CC) $(CFLAGS) -c driver.c

scanner.o: scanner.c scanner.h
	$(CC) $(CFLAGS) -c scanner.c

semantic.o: semantic.c scanner.h ast.h gen_code.h
	$(CC) $(CFLAGS) -c semantic.c

ast-helper.o: ast-helper.c ast.h
	$(CC) $(CFLAGS) -c ast-helper.c

ast-print.o: ast-print.c ast.h
	$(CC) $(CFLAGS) -c ast-print.c

mips_gen.o: mips_gen.c gen_code.h ast.h
	$(CC) $(CFLAGS) -c mips_gen.c

gen_code.o: gen_code.c gen_code.h
	$(CC) $(CFLAGS) -c gen_code.c

clean:
	rm -f *.o compile *.s
