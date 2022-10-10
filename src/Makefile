CXX=clang++
# NOTE(__LUNA__): Is there anyway to add global debug flag to enable -O3 and NDEBUG?
CXXFLAGS=-std=c++17 -Wall -Wextra -pedantic -DNDEBUG -O3 `pkg-config glfw3 --cflags`
LIBS=-lVulkan `pkg-config glfw3 --static --libs`
# NOTE(__LUNA__): UNAME_S doesn't exist on Windows, would ideally need to detect that first | Oh and also no pkg-config
UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S), Linux)
	LIBS+=-ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
endif
SRC=main.cpp App.cpp Window.cpp Input.cpp
TARGET=Wulkan

all: $(TARGET)

# NOTE(__LUNA__): Filter out NDEBUG and 03 for debug mode
debug: CXXFLAGS:=$(filter-out -DNDEBUG, $(CXXFLAGS))
debug: CXXFLAGS:=$(filter-out -O3, $(CXXFLAGS))
debug: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(LIBS) $(SRC) -o $(TARGET)

clean:
	rm -f *.o $(TARGET)
