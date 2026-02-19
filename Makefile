bin/libmystd.a: mystd/stdlib.c mystd/stdio.c mystd/tracker.c mystd/string.c
	gcc -O3 -I. -std=gnu23 -c mystd/stdlib.c 	-o bin/stdlib.o
	gcc -O3 -I. -std=gnu23 -c mystd/stdio.c 	-o bin/stdio.o
	gcc -O3 -I. -std=gnu23 -c mystd/tracker.c 	-o bin/tracker.o
	gcc -O3 -I. -std=gnu23 -c mystd/string.c 	-o bin/string.o
	ar rcs bin/libmystd.a bin/stdlib.o bin/stdio.o bin/tracker.o bin/string.o