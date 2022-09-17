
BIN_DIR:=./bin
SRC_DIRS:=./src
LIB_DIR:=./src/libs
MAIN_DIR:=./src

# All sources files
SRCS:=$(shell find $(SRC_DIRS) -name *.c)
# All objects files
LIBS:=$(shell find $(LIB_DIR) -name *.c) 
OBJS:=$(LIBS:$(LIB_DIR)/%.c=$(BIN_DIR)/%.o)

DEPS:=$(OBJS:.o=.d)


CC:=gcc
CFLAGS:=-O3 -Wall -Wextra -g -pg -I$(LIB_DIR)

# recipes 
all :

% : $(OBJS) $(MAIN_DIR)/%.c 
	$(CC) $^ $(LDFLAGS) $(CFLAGS) -o $(BIN_DIR)/$@.out


$(BIN_DIR)/%.o: $(LIB_DIR)/%.c $(LIB_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: echo
echo :
	echo $(SRCS)
	echo $(OBJS)
	echo $(INC_DIRS)
	echo $(CFLAGS)
	
.PHONY: clear
clear :
	rm -r $(BIN_DIR)

