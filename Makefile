CC=clang
CFLAGS=-g `llvm-config --cflags`
LD=clang++
LDFLAGS= `llvm-config --cxxflags --ldflags --libs all --system-libs`

all: sum.out factorial.out nth_prime.out

sum.o: sum.c
	$(CC) $(CFLAGS) -c $<

sum.out: sum.o
	$(LD) $< $(LDFLAGS) -o $@

sum.bc: sum
	./sum 0 0

sum.ll: sum.bc
	llvm-dis $<

factorial.o: factorial.c
	$(CC) $(CFLAGS) -c $<

factorial.out: factorial.o
	$(LD) $< $(LDFLAGS) -o $@

factorial.bc: factorial
	./factorial 0

factorial.ll: factorial.bc
	llvm-dis $<

nth_prime.o: nth_prime.c
	$(CC) $(CFLAGS) -c $<

nth_prime.out: nth_prime.o
	$(LD) $< $(LDFLAGS) -o $@

nth_prime.bc: nth_prime
	./nth_prime 0

nth_prime.ll: nth_prime.bc
	llvm-dis $<

clean:
	rm -f *.out *.o *.bc *.ll