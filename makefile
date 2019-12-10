AS = yasm

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = test

ASM_EXT = asm
OBJ_EXT = o
LIST_EXT = list

SOURCES = $(wildcard $(SRC_DIR)/*$(ASM_EXT))
OBJECTS = $(patsubst $(SRC_DIR)/%.$(ASM_EXT), $(OBJ_DIR)/%.$(OBJ_EXT), $(SOURCES))
MAIN_OBJ = $(OBJ_DIR)/main.$(OBJ_EXT)
LIBS = $(OBJ_DIR)/gf28lib.o
BINARY = $(BIN_DIR)/aes

TEST = $(TEST_DIR)/$(BIN_DIR)/test
TEST_OBJ = $(TEST_DIR)/$(OBJ_DIR)/test.o
TEST_SRC = $(TEST_DIR)/$(SRC_DIR)/test.cpp

ASFLAGS = -f elf64 -g dwarf2
CXXFLAGS = -c -g --std=c++11
LDFLAGS = -no-pie

all: $(BINARY) $(TEST)

test: $(TEST)

$(TEST): $(TEST_OBJ) $(OBJECTS)
	mkdir -p $(TEST_DIR)/$(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $(TEST) $(TEST_OBJ) $(OBJECTS) 

$(BINARY): $(OBJECTS) $(MAIN_OBJ)
	mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $(BINARY) $(OBJECTS) $(MAIN_OBJ)

$(TEST_OBJ) : $(TEST_SRC)
	mkdir -p $(TEST_DIR)/$(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(MAIN_OBJ) : $(SRC_DIR)/main.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(OBJ_DIR)/%.$(OBJ_EXT) : $(SRC_DIR)/%.$(ASM_EXT)
	mkdir -p $(OBJ_DIR)
	$(AS) $(ASFLAGS) -o $@ $< -l $(patsubst %.$(OBJ_EXT), %.$(LIST_EXT), $@)

.PHONY: clean_all
clean_all:
	$(RM) $(BINARY)
	$(RM) $(OBJ_DIR)/*.$(OBJ_EXT)
	$(RM) $(OBJ_DIR)/*.$(LIST_EXT)
	$(RM) $(TEST)
	$(RM) $(TEST_OBJ)

.PHONY: clean
clean:
	$(RM) $(BINARY)
	$(RM) $(OBJ_DIR)/*.$(OBJ_EXT)
	$(RM) $(OBJ_DIR)/*.$(LIST_EXT)

.PHONY: clean_test
clean_test:
	$(RM) $(TEST)
	$(RM) $(TEST_OBJ)
