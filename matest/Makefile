DIR = ~/bin/libmathsart/
matest: matest.o libmathsart.o
	gcc-apple-4.2 -O3 -Wall -Wextra -o matest libmathsart.o matest.o -lpng

matest.o: matest.c $(DIR)libmathsart.h 
	gcc-apple-4.2 -O3 -Wall -Wextra -c matest.c 

libmathsart.o: $(DIR)libmathsart.c $(DIR)libmathsart.h 
	gcc-apple-4.2 -O3 -Wall -Wextra -c $(DIR)libmathsart.c 
