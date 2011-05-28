CXX=g++-4.6 -std=c++0x
CXXFLAGS=-O3 -Wall -Wextra

all: simple_send_test

%.o: %.c++
	$(CXX) $(CXXFLAGS) $^ -c -o $@

simple_send_test: simple_send_test.c++ Raw_Ethernet.o
	$(CXX) $(CXXFLAGS) $^ -o $@
