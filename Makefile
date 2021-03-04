TARGET = run

CXX = g++
CXXFLAGS = -std=c++0x -Wall -g
OBJECTS = efficientnet-lite0.o conv.o activation.o utils.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)
	rm -f $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET) *~