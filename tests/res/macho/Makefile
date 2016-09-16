all:
	clang++ main.cc -o macho_main
	clang++ -c main.cc -o macho_main.o
	clang++ -m32 main.cc -o macho_main_32
	clang++ -m32 -c main.cc -o macho_main_32.o
	clang++ -arch i386 -arch x86_64 main.cc -o macho_main_32_64


clean:
	rm -fv macho_*
