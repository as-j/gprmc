gprmc: gprmc.c 
	$(CC) -Wall -ansi -o $@ $<

clean:
	rm -f gprmc
	
test: