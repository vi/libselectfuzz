libselectfuzz.so.0.0.0: override.c Makefile
		gcc -shared -Wall -fPIC -o libselectfuzz.so.0.0.0 override.c -ldl
		ln -fs libselectfuzz.so.0.0.0 libselectfuzz.so.0
		ln -fs libselectfuzz.so.0.0.0 libselectfuzz.so
