1: overflow_lib include/overflow.h include/self_errors.h
	gcc -g ./src/first.c -I./include -L./impl -lOverflow -Wl,-rpath,'$$ORIGIN/../impl' -o ./bin/first

2: file_lib overflow_lib include/files.h include/overflow.h include/self_errors.h
	gcc -g ./src/second.c -I./include -L./impl -lFiles -lOverflow -Wl,-rpath,'$$ORIGIN/../impl' -o ./bin/second

3: overflow_lib patterns_lib include/overflow.h include/patterns.h include/self_errors.h
	gcc -g ./src/third.c -I./include -L./impl -lOverflow -lPatterns -Wl,-rpath,'$$ORIGIN/../impl' -o ./bin/third

5:
	gcc -g ./src/five.c -o ./bin/five

6:
	gcc -g ./src/six.c -o ./bin/six

7: include/self_errors.h
	gcc -g ./src/seven.c -I./include -lm -o ./bin/seven

8: include/self_errors.h
	gcc -g ./src/eight.c -I./include -o ./bin/eight

9: include/self_errors.h
	gcc -g ./src/nine.c -I./include -o ./bin/nine

10: include/self_errors.h
	gcc -g ./src/ten.c -I./include -o ./bin/ten

overflow_lib: include/overflow.h
	gcc -fPIC -shared lib/overflow.c -I./include -o ./impl/libOverflow.so

file_lib: include/files.h include/self_errors.h
	gcc -fPIC -shared lib/files.c -I./include -o ./impl/libFiles.so

patterns_lib: include/patterns.h include/self_errors.h
	gcc -g -fPIC -shared lib/search_patterns.c -I./include -o ./impl/libPatterns.so
