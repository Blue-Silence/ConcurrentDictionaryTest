main :
	g++ -I/home/usera/CPP\ LIB/folly/installed/folly/include/ *.cpp -L/home/usera/CPP\ LIB/folly/installed/folly/lib/ -l:libfolly.a -lglog -lgflags -lfmt

clean :
	rm *.out