include ../../Makefile.mk

MODULES := exception pf

CPPFLAGS += $(patsubst %,-I../../%,$(MODULES))

# include the description for each module
include $(patsubst %,../../%/module_test.mk,$(MODULES))
SRC := $(patsubst %,../../%,$(SRC))

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC)))

# link the program
PageHandleTest: $(OBJ) PageHandleTest.o
	$(CXX) -o $@ $^

BufferPoolTest: $(OBJ) BufferPoolTest.o
	$(CXX) -o $@ $^

.PHONY: all
all: PageHandleTest BufferPoolTest

.PHONY: clean
clean:
	$(RM) $(OBJ) PageHandleTest BufferPoolTest *.o
