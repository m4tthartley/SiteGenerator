
clang -g -O0 ../code/main.cc -o ../build/websitegenerator\
	-Wno-c++11-compat-deprecated-writable-strings\
	-Wno-format-security -Wno-null-dereference\
	-Wno-format

