CXX = g++
CXXFLAGS = -std=c++17 -Wall
LIBS = -lgpiod
SOURCES = main.cpp serial_port.cpp sms_utils.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = gardener

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

main.o: main.cpp serial_port.h sms_utils.h time_consts.h
serial_port.o: serial_port.cpp serial_port.h
sms_utils.o: sms_utils.cpp sms_utils.h serial_port.h time_consts.h

clean:
	rm -f $(OBJECTS) $(TARGET)

install:
	cp -f gardener /usr/bin/

.PHONY: all clean
