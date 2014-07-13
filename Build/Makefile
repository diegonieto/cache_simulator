CC     = gcc
LIBS   = sim_cache.c
OBJS   = cache_simulator test
CFLAGS = -O3 

all: $(OBJS)

cache_simulator:
	$(CC) -o $@ ../$@.c ../$(LIBS) $(CFLAGS)
	
test:
	$(CC) -o $@ ../$@.c ../$(LIBS) $(CFLAGS)
	  
clean:
	rm $(OBJS)