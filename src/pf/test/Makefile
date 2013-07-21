include ../../Makefile.mk

MODULES := exception pf

TESTSRC := PageHandleTest.cpp

CPPFLAGS += $(patsubst %,-I../../%,$(MODULES))

# include the description for each module
include $(patsubst %,../../%/module_test.mk,$(MODULES))
SRC := $(patsubst %,../../%,$(SRC)) $(TESTSRC)

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC)))

# link the program
PageHandleTest: $(OBJ)
	$(CXX) -o $@ $^

.PHONY: all
all: PageHandleTest 

.PHONY: clean
clean:
	$(RM) $(OBJ) PageHandleTest
