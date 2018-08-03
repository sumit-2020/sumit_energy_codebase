program_NAME := safari_mc_test
program_C_SRCS := $(wildcard src/*.c)
program_CXX_SRCS := $(wildcard src/*.cpp)
program_C_OBJS := ${program_C_SRCS:.cpp=.o}
program_CXX_OBJS := ${program_CXX_SRCS:.cpp=.o}
program_OBJS := $(program_C_OBJS) $(program_CXX_OBJS)
program_INCLUDE_DIRS := ./src
program_LIBRARY_DIRS :=
program_LIBRARIES := riffa
INV= -DINVERT_PATT=1 -DINVERT_HALF=0
TIMINGS= -DDEF_TRCD=5 -DDEF_TRP=5 -DGUARDBAND=0 -DRD_GUARDBAND=0
CPPFLAGS += -std=c++11 -O3 $(TIMINGS) $(INV)

CPPFLAGS += $(foreach includedir,$(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))

CC=g++

.PHONY: all clean distclean

all: $(program_NAME)

debug: CPPFLAGS += -DDEBUG -g
debug: $(program_NAME)

$(program_NAME): $(program_OBJS)
	$(CC) $(CPPFLAGS) $(program_OBJS) -o bin/$(program_NAME) $(LDFLAGS)

clean:
	@- $(RM) bin/$(program_NAME)
	@- $(RM) $(program_OBJS)

distclean: clean
