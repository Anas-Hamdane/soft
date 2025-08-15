SRC        := ./src
BUILD      := ./build
INCLUDE    := ./include

CXX        := clang++
CXXFLAGS   := -std=c++17 -Wall -Wextra -save-temps -I$(INCLUDE) -g

TARGET     := $(BUILD)/soft

SOURCES := $(SRC)/main.cpp

OBJECTS := $(SOURCES:$(SRC)/%.cpp=$(BUILD)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(CXXFLAGS)

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	@mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
