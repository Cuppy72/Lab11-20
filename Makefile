1: overflow_lib include/overflow.h include/self_errors.h
	gcc -g ./src/first.c -I./include -L./impl -lOverflow -Wl,-rpath,'$$ORIGIN/../impl' -o ./bin/first

2: file_lib overflow_lib include/files.h include/overflow.h include/self_errors.h
	gcc -g ./src/second.c -I./include -L./impl -lFiles -lOverflow -Wl,-rpath,'$$ORIGIN/../impl' -o ./bin/second

3: overflow_lib patterns_lib include/overflow.h include/patterns.h include/self_errors.h
	gcc -g ./src/third.c -I./include -L./impl -lOverflow -lPatterns -Wl,-rpath,'$$ORIGIN/../impl' -o ./bin/third


8: include/self_errors.h
	gcc -g ./src/eight.c -I./include -o ./bin/eight

overflow_lib: include/overflow.h
	gcc -fPIC -shared lib/overflow.c -I./include -o ./impl/libOverflow.so

file_lib: include/files.h include/self_errors.h
	gcc -fPIC -shared lib/files.c -I./include -o ./impl/libFiles.so

patterns_lib: include/patterns.h include/self_errors.h
	gcc -g -fPIC -shared lib/search_patterns.c -I./include -o ./impl/libPatterns.so
