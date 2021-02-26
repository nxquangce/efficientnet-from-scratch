TARGET = run

CXX = g++
CXXFLAGS = -std=c++0x -Wall
OBJECTS = efficientnet-lite0.o conv.o activation.o utils.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET) *~