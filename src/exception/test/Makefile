include ../../Makefile.mk

MODULES := exception

SRC := 

CPPFLAGS += $(patsubst %,-I../../%,$(MODULES))

# include the description for each module
include $(patsubst %,../../%/module_test.mk,$(MODULES))
SRC := $(patsubst %,../../%,$(SRC))

SRC += test.cpp

OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC)))

# link the program
test: $(OBJ)
	$(CXX) -o $@ $^

.PHONY: all
all: test 

.PHONY: clean
clean:
	$(RM) $(OBJ) test
