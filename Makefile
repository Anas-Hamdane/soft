SRC        := ./src
INCLUDE    := ./include
PCH_HEADER := $(INCLUDE)/stl.h

BUILD      := ./build
PCH        := $(PCH_HEADER).gch

CXX        := clang++
CXXFLAGS   := -std=c++23 -Wall -Wextra -I$(INCLUDE) -g

TARGET     := $(BUILD)/soft

RSS        := $(SRC)/main.cpp \
								$(SRC)/file.cpp \
								$(SRC)/lexer.cpp \
								$(SRC)/opts.cpp

OBJS := $(RSS:$(SRC)/%.cpp=$(BUILD)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) $(PCH) | $(BUILD)
	$(CXX) $(OBJS) -o $(TARGET)

$(BUILD)/%.o: $(SRC)/%.cpp $(PCH) | $(BUILD)
	$(CXX) -c $< -o $@ -include-pch $(PCH) $(CXXFLAGS)

$(PCH): $(PCH_HEADER) | $(BUILD)
	$(CXX) -x c++-header $< -o $@ $(CXXFLAGS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
