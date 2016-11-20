gprmc: gprmc.c 
	$(CC) -Wall -std=c99 -D_GNU_SOURCE -o $@ $<

clean:
	rm -f gprmc
	
FILES = 4sentence mixed broken resync limit
$(FILES):
	@echo -n Checking $@...
	@./gprmc < data/$(@).txt > out.txt && diff -u out.txt data/valid/$(@)_out.txt
	@rm -f out.txt
	@echo 	OK!

check: gprmc $(FILES)

	