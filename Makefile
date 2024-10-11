CXX = g++
CXXFLAGS = `pkg-config --cflags cairo` -Wall -Wextra -pedantic
LDFLAGS = -lXfixes -lXcomposite -lX11 -lXrandr `pkg-config --libs cairo`

TARGET = clipscreen
SOURCES = clipscreen.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
