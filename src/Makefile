CXX=clang++
# NOTE(__LUNA__): Is there anyway to add global debug flag to enable -O3 and NDEBUG?
WFLAGS=-std=c++17 -Wall -Wextra -pedantic -DNDEBUG -O3
CXXFLAGS=`pkg-config vulkan glfw3 --cflags`
LIBS=`pkg-config vulkan glfw3 --libs`
# NOTE(__LUNA__): UNAME_S doesn't exist on Windows, would ideally need to detect that first | Oh and also no pkg-config
UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S), Darwin)
	LIBS+=-framework Cocoa -framework IOKit
endif # ($(UNAME_S), Darwin)
ifeq ($(UNAME_S), Linux)
	LIBS+=-ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
endif # ($(UNAME_S), Linux)
SRC=main.cpp App.cpp Window.cpp Input.cpp
TARGET=Wulkan

all: $(TARGET)

# NOTE(__LUNA__): Filter out NDEBUG and 03 for debug mode
debug: WFLAGS:=$(filter-out -DNDEBUG, $(WFLAGS))
debug: WFLAGS:=$(filter-out -O3, $(WFLAGS))
debug: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(WFLAGS) $(CXXFLAGS) $(LIBS) $(SRC) -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
