CXX := g++

BASE_CPPFLAGS = -I./include -D_ASM -std=c++17 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla

NORMAL_CPPFLAGS = $(BASE_CPPFLAGS) -O2
DEBUG_CPPFLAGS  = $(BASE_CPPFLAGS) -ggdb3 -O0 -D_DEBUG -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

TARGET := assembler

SOURCES := src/Assembler/main.cpp src/Assembler/assembler.cpp lib/Processing/FileRWUtils.cpp src/errors.cpp lib/errors.cpp

NORMAL_OBJECTS := obj/normal/src/Assembler/main.o obj/normal/src/Assembler/assembler.o obj/normal/lib/Processing/FileRWUtils.o obj/normal/lib/errors.o

DEBUG_OBJECTS  := obj/debug/src/Assembler/main.o obj/debug/src/Assembler/assembler.o  obj/debug/lib/Processing/FileRWUtils.o  obj/debug/lib/errors.o

all: $(TARGET)

$(TARGET): $(NORMAL_OBJECTS)
	$(CXX) $(NORMAL_CPPFLAGS) -o $@ $^

debug: $(TARGET)_debug

$(TARGET)_debug: $(DEBUG_OBJECTS)
	$(CXX) $(DEBUG_CPPFLAGS) -o $@ $^

obj/normal/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(NORMAL_CPPFLAGS) -c $< -o $@

obj/debug/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(DEBUG_CPPFLAGS) -c $< -o $@

clean:
	rm -rf obj $(TARGET) $(TARGET)_debug

.PHONY: all debug clean
