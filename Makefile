SRC        := ./src
INCLUDE    := ./include
PCH_HEADER := $(INCLUDE)/stl.h

BUILD      := ./build
DATA       := ./build/data
IR         := ./build/ir
CODEGEN    := ./build/codegen
PCH        := $(PCH_HEADER).gch

CXX        := clang++
CXXFLAGS   := -std=c++23 -Wall -Wextra -I$(INCLUDE) -g

TARGET     := $(BUILD)/soft

RSS        := $(SRC)/main.cpp                   \
								$(SRC)/common.cpp               \
								$(SRC)/opts.cpp                 \
								$(SRC)/file.cpp                 \
								$(SRC)/lexer.cpp                \
								$(SRC)/parser.cpp               \
								$(SRC)/data/Type.cpp            \
								$(SRC)/data/Constant.cpp        \
								$(SRC)/data/Slot.cpp            \
								$(SRC)/data/Value.cpp           \
								$(SRC)/ir/ir.cpp                \
								$(SRC)/ir/Instruction.cpp       \
								$(SRC)/ir/Program.cpp           \
								$(SRC)/codegen/Storage.cpp      \
								$(SRC)/codegen/DataLabel.cpp    \
								$(SRC)/codegen/codegen.cpp      \

OBJS := $(RSS:$(SRC)/%.cpp=$(BUILD)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) $(PCH) | $(BUILD) $(DATA) $(IR) $(CODEGEN)
	$(CXX) $(OBJS) -o $(TARGET)

$(BUILD)/%.o: $(SRC)/%.cpp $(PCH) | $(BUILD) $(DATA) $(IR) $(CODEGEN)
	$(CXX) -c $< -o $@ -include-pch $(PCH) $(CXXFLAGS)

$(PCH): $(PCH_HEADER)
	$(CXX) -x c++-header $< -o $@ $(CXXFLAGS)

$(BUILD):
	mkdir -p $(BUILD)

$(DATA):
	mkdir -p $(DATA)

$(IR):
	mkdir -p $(IR)

$(CODEGEN):
	mkdir -p $(CODEGEN)

clean:
	rm -rf $(BUILD)
