gprmc: gprmc.c 
	$(CC) -Wall -ansi -o $@ $<

clean:
	rm -f gprmc
	
FILES = 4sentence mixed broken resync
$(FILES):
	@echo -n Checking $@...
	@./gprmc < data/$(@).txt > out.txt && cmp out.txt data/valid/$(@)_out.txt
	@rm -f out.txt
	@echo 	OK!

check: gprmc $(FILES)

	