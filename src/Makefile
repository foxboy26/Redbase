# each Makefile.mk in the sub folder should specify the following variables
# SRC +=


# each module will add to this
SRC :=

# extra libraries if required
LIBS :=

# add module name
MODULES  := main pf log 

CXX      := clang++ 
CXXFLAGS := -std=c++11 -stdlib=libc++ -Weverything -g
CPPFLAGS += $(patsubst %,-I%,$(MODULES))

# include the description for each module
include $(patsubst %,%/module.mk,$(MODULES))

# determine the object files
OBJ := $(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) $(patsubst %.y,%.o,$(filter %.y,$(SRC)))

# link the program
program: $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LIBS)

# include the include dependencies
include $(OBJ:.o=.d)

# calculate include dependencies
%.d: %.cpp
	./depend.sh `dirname $*.cpp` $(CFLAGS) $*.cpp > $@

.PHONY: all
all: program

.PHONY: clean
clean:
	$(RM) $(OBJ) $(programs) $(dependencies)
