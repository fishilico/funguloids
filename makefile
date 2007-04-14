# "Those Funny Funguloids!" Makefile

.PHONY: all clean veryclean rebuild

CC = g++
TARGET = ./bin/funguloids
MARCH = pentium


# Build settings
CFLAGS = -MMD -O3 -march=$(MARCH) -Wall `pkg-config OGRE --cflags` -s
LDFLAGS = `pkg-config OGRE --libs` -lOIS -lfmodex -llua5.1 -s


# Source and object files
SOURCES = $(wildcard src/*.cpp)
OBJS = $(SOURCES:.cpp=.o)
OBJS := $(subst src/,obj/,$(OBJS))

# Include directories
# (modify to reflect your own Lua and FMOD Ex installations)
INCLUDES = -I/usr/include/lua5.1 -I./include -I./include/SimpleIni -I/usr/local/include/fmodex


# Targets
all: $(TARGET)

# Check dependancies
DEPS = $(subst .o,.d,$(OBJS))
-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)


clean:
	rm -rf $(OBJS) $(TARGET)

veryclean:
	rm -rf $(OBJS) $(TARGET) $(DEPS)

rebuild: veryclean all

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DIR_DEFINES) -c $< -o $@

