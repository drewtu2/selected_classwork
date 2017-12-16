###############################################################################
# Makefile Template
# Andrew Tu
#
###############################################################################


###############################################################################
# Important Variables
###############################################################################
SRC_DIR := src
RELEASE_DIR := release
DEBUG_DIR := debug

# Compilers
CXX := gcc
#CXX_FLAGS := -std=c99 
CXX_FLAGS := -std=c99 -D_XOPEN_SOURCE=700

RELEASE_FLAGS := -o3
DEBUG_FLAGS := -g3

INCLUDES := -I include
LIBS :=

###############################################################################
# Sources List + Object List Creation
###############################################################################

# A list of source files for the executable we want to build
EXEC_SRCS := \
  $(SRC_DIR)/svec.c\
  $(SRC_DIR)/nush.c\
  $(SRC_DIR)/tokens.c

# An self generating list of objects for each of the objects needed in the 
# executable
RELEASE_EXEC_OBJS += $(patsubst $(SRC_DIR)/%.c, $(RELEASE_DIR)/%.o, \
	     $(filter $(SRC_DIR)/%.c, $(EXEC_SRCS)))
DEBUG_EXEC_OBJS += $(patsubst $(SRC_DIR)/%.c, $(DEBUG_DIR)/%.o, \
	     $(filter $(SRC_DIR)/%.c, $(EXEC_SRCS)))

.PHONY: \
  all \
  release \
  debug \
  clean \
  tests

###############################################################################
# Build Commands 
###############################################################################
all : \
  debug
  #release

release :\
 	$(RELEASE_DIR)/Driver

debug :\
  	$(DEBUG_DIR)/Driver
	mv $(DEBUG_DIR)/Driver nush

# The build sequence for the particular executable we want to build. 
$(RELEASE_DIR)/Driver: $(RELEASE_EXEC_OBJS)
	@echo $(RELEASE_EXEC_OBJS)
	$(CXX) $(CXX_FLAGS) $(RELEASE_FLAGS) $(INCLUDES) $(LIBS) -o $@ $^

$(DEBUG_DIR)/Driver: $(DEBUG_EXEC_OBJS)
	@echo $(DEBUG_EXEC_OBJS)
	$(CXX) $(CXX_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) $(LIBS) -o $@ $^

clean :
	@rm -rf $(DEBUG_DIR) $(RELEASE_DIR) tmp *.dSYM *.out nush *.plist *.tmp

tests : 
	perl test.pl

###############################################################################
# Dependencies 
###############################################################################

# Build the .o file for each src file
$(RELEASE_DIR)/%.o : $(SRC_DIR)/%.c | release_directory
	@echo $(RELEASE_EXEC_OBJS)
	$(CXX) $(CXX_FLAGS) $(RELEASE_FLAGS) $(INCLUDES) -c $< -o $@

$(DEBUG_DIR)/%.o : $(SRC_DIR)/%.c | debug_directory
	$(CXX) $(CXX_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) -c $< -o $@

# Autogen dependencies
$(RELEASE_DIR)/%.d : $(SRC_DIR)/%.c
	@echo "Running Autogen..."
	@set -e;\
	rm -f $@;\
	$(CXX) -MM $(CXX_FLAGS) $(INCLUDES) $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,$(RELEASE_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(DEBUG_DIR)/%.d : $(SRC_DIR)/%.c
	@echo "Running Autogen..."
	@set -e;\
	rm -f $@;\
	$(CXX) -MM $(CXX_FLAGS) $(INCLUDES) $< > $@.$$$$;\
	sed 's,\($*\)\.o[ :]*,$(DEBUG_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Create the build directories
release_directory :
	@echo "Make Release Dir"
	@mkdir -p $(RELEASE_DIR)

debug_directory :
	@mkdir -p $(DEBUG_DIR)

-include $(OBJS:.o=.d)
