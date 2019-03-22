args: examples/args.cpp include/cake/args.hpp
	clang++ -std=c++14 -Iinclude examples/args.cpp -o $@
