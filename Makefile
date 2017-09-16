CC=clang
CFLAGS=-g `llvm-config --cflags`
LD=clang++
LDFLAGS= `llvm-config --cxxflags --ldflags --libs all --system-libs`

all: sum factorial

sum.o: sum.c
	$(CC) $(CFLAGS) -c $<

sum: sum.o
	$(LD) $< $(LDFLAGS) -o $@ 

sum.bc: sum
	./sum 0 0

sum.ll: sum.bc
	llvm-dis $<

factorial.o: factorial.c
	$(CC) $(CFLAGS) -c $<

factorial: factorial.o
	$(LD) $< $(LDFLAGS) -o $@ 

factorial.bc: factorial
	./factorial 0

factorial.ll: factorial.bc
	llvm-dis $<

clean:
	rm -f sum.o sum sum.bc sum.ll factorial.o factorial factorial.bc factorial.ll 
