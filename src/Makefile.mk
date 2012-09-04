# p
# each file should specify the following variables
#programs     :=
#sources      :=
#libraries    :=
#extra_clean  :=
#
#modules      :=
#include_dirs :=



#obj_dir      := obj
#dep_dir      := dep
objects      = $(subst .cpp,.o,$(sources))
dependencies = $(subst .cpp,.d,$(sources))
#dependencies = $(addprefix $(dep_dir)/,$(subst .cpp,.d,$(sources)))

CC       := $(CXX)
CXXFLAGS := -Wall -Werror -g
CPPFLAGS += $(addprefix -I,$(include_dirs))
vpath %.h $(include_dirs)

RM  := rm -f
MV  := mv -f
SED := sed

.PHONY: all
all: $(programs)

.PHONY: clean
clean:
	$(RM) $(objects) $(programs) $(dependencies)

ifneq ($(MAKECMDGOALS), clean)
	-include $(dependencies)
endif

# $(call make-depend source, object, depend)
define make-depend
	$(CXX) -MM            \
				 -MF $3         \
				 -MP            \
				 -MT $2         \
				 $(CXXFLAGS)    \
				 $(CPPFLAGS)    \
				 $(TARGET_ARCH) \
				 $1
endef

%.o: %.cpp
	$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<
