<<<<<<< HEAD
CC=/usr/bin/g++
CFLAGS=-Wall -Werror -std=c++17 -g3 -ggdb -O0 -fno-strict-aliasing -fno-omit-frame-pointer -D_GLIBCXX_USE_CXX11_ABI=0
#CFLAGS += -fsanitize=address -fno-stack-protector -fno-omit-frame-pointer
=======
CC=g++

CFLAGS=-Wall -Werror -std=c++11 -g3 -ggdb -O0 -fno-strict-aliasing -I/usr/local/include -I/home/u2021000884/include -L/usr/local/lib -L/home/u2021000884/lib -D_GLIBCXX_USE_CXX11_ABI=0 -fno-omit-frame-pointer -D_GLIBCXX_USE_CXX11_ABI=0

# CFLAGS += -fsanitize=address -fno-stack-protector -fno-omit-frame-pointer
>>>>>>> 8ee691f8bc5012b01a09fa4ed4cd44586f4b7b9d
NNMSG=./nanomsg-0.5-beta
#RALLOC = ./rlib/lib

.SUFFIXES: .o .cpp .h .cc

SRC_DIRS = ./ ./benchmarks/ ./client/ ./concurrency_control/ ./storage/ ./transport/ ./system/ ./statistics/#./unit_tests/
DEPS = -I. -I./benchmarks -I./client/ -I./concurrency_control -I./storage -I./transport -I./system -I./statistics #-I./unit_tests


CFLAGS += $(DEPS) -D NOGRAPHITE=1 -Wno-sizeof-pointer-memaccess 
LDFLAGS = -Wall -L. -L$(NNMSG) -Wl,-rpath -pthread -lrt -lnanomsg -lanl -lcurl -lpthread
LDFLAGS += $(CFLAGS)
LIBS =


DB_MAINS = ./client/client_main.cpp ./system/sequencer_main.cpp ./unit_tests/unit_main.cpp
CL_MAINS = ./system/main.cpp ./system/sequencer_main.cpp ./unit_tests/unit_main.cpp
UNIT_MAINS = ./system/main.cpp ./client/client_main.cpp ./system/sequencer_main.cpp

CPPS_DB = $(foreach dir,$(SRC_DIRS),$(filter-out $(DB_MAINS), $(wildcard $(dir)*.cpp)))
CPPS_CL = $(foreach dir,$(SRC_DIRS),$(filter-out $(CL_MAINS), $(wildcard $(dir)*.cpp)))
CPPS_UNIT = $(foreach dir,$(SRC_DIRS),$(filter-out $(UNIT_MAINS), $(wildcard $(dir)*.cpp)))

#CPPS = $(wildcard *.cpp)
OBJS_DB = $(addprefix obj/, $(notdir $(CPPS_DB:.cpp=.o)))
OBJS_CL = $(addprefix obj/, $(notdir $(CPPS_CL:.cpp=.o)))
OBJS_UNIT = $(addprefix obj/, $(notdir $(CPPS_UNIT:.cpp=.o)))

#NOGRAPHITE=1

all: rundb runcl
#unit_test

.PHONY: deps_db
deps:$(CPPS_DB)
	$(CC) $(CFLAGS) -MM $^ > obj/deps
	sed '/^[^ ]/s/^/obj\//g' obj/deps > obj/deps.tmp
	mv obj/deps.tmp obj/deps
-include obj/deps

unit_test : $(OBJS_UNIT)
#	$(CC)   -o $@ $^ $(LDFLAGS) $(LIBS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
./obj/%.o: transport/%.cpp
#	$(CC)   -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
	$(CC) -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
./obj/%.o: unit_tests/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: benchmarks/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: storage/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: system/%.cpp
	$(CC) -c -DSTATS_ENABLE=false $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: concurrency_control/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: client/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<


rundb : $(OBJS_DB)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
#	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
./obj/%.o: transport/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
#	$(CC) -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
#./deps/%.d: %.cpp
#	$(CC) -MM -MT $*.o -MF $@ $(CFLAGS) $<
./obj/%.o: benchmarks/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: storage/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: system/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: statistics/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: concurrency_control/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: client/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<


runcl : $(OBJS_CL)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
#	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
./obj/%.o: transport/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
#	$(CC) -c $(CFLAGS) $(INCLUDE) $(LIBS) -o $@ $<
#./deps/%.d: %.cpp
#	$(CC) -MM -MT $*.o -MF $@ $(CFLAGS) $<
./obj/%.o: benchmarks/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: storage/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: system/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: statistics/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: concurrency_control/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: client/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
./obj/%.o: %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

.PHONY: clean
clean:
	rm -f obj/*.o obj/.depend rundb runcl runsq unit_test
