CXXFLAGS = -g -Wall -Werror -std=c++17
LDLIBS =

PRGM  = Main
SRCS := $(wildcard *.cpp)
HDRS := $(wildcard *.h)
OBJSH := $(HDRS:.h=.o)
DEPSH := $(OBJSH:.o=.d)

OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

.PHONY: build run clean

build: $(PRGM)

$(PRGM): $(OBJS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(PRGM)
	./$(PRGM)

clean:
	rm -rf $(OBJS) $(OBJSH) $(DEPS) $(DEPSH)
	rm -rf $(PRGM)
