CC=gcc
CFLAGS=-fsanitize=address -Wvla -Wall -Werror -g -Os -s -std=gnu11 -lasan

vm_x2017: vm_x2017.c
	$(CC) $(CFLAGS) $^ -o $@ "parser.c"
	export ASAN_OPTIONS=verify_asan_link_order=0

objdump_x2017: objdump_x2017.c
	$(CC) $(CFLAGS) $^ -o $@ "parser.c"
	export ASAN_OPTIONS=verify_asan_link_order=0

tests:
	echo "run_tests does everything"

run_tests:
	chmod a+x test.sh
	chmod a+x tests/encode_all_files.sh
	export ASAN_OPTIONS=verify_asan_link_order=0
	./test.sh
