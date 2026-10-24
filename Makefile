1: overflow_lib include/overflow.h
	gcc ./src/first.c -I./include -L./impl -lOverflow -Wl,-rpath,'$$ORIGIN/../impl' -o ./bin/first

2: overflow_lib include/overflow.h file_lib include/files.h
	gcc ./src/second.c -I./include -L./impl -lOverflow -lFiles -Wl,-rpath,'$$ORIGIN/../impl' -o ./bin/second

overflow_lib: include/overflow.h
	gcc -fPIC -shared lib/overflow.c -I./include -o ./impl/libOverflow.so
file_lib: include/files.h
	gcc -fPIC -shared lib/files.c -I./include -o ./impl/libFiles.so

clean_1:
	rm -f ./bin/first ./impl/libOverflow.so

