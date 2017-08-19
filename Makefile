export
CC=gcc
CXX=g++
LD=g++
AR=gcc-ar
CFLAGS=-std=gnu11 -c -Wall -Werror
CFLAGS_RELAXED=-std=gnu11 -c
CXXFLAGS=-std=c++14 -c -Wall -Werror
CXXFLAGS_RELAXED=-std=c++14 -c
LDFLAGS=
ARFLAGS=rc

DBG=-g
OPT=-flto -O3 $(VECTOR)

ifdef RELEASE
	CFLAGS += $(OPT)
	CFLAGS_RELAXED += $(OPT)
	CXXFLAGS += $(OPT)
	CXXFLAGS_RELAXED += $(OPT)
	LDFLAGS += $(OPT)
else
	CFLAGS += $(DBG)
	CFLAGS_RELAXED += $(DBG)
	CXXFLAGS += $(DBG)
	CXXFLAGS_RELAXED += $(DBG)
endif

LIBS=-Lbuild -lsdr_stream -lm -pthread
LIBS_UI=-Lbuild "-Wl,-rpath=$(shell realpath build)" -lsdr_ui

.PHONY: default all clean stream ui

default: all

all: build/constellation build/signal build/stream_debug build/throttle

build:
	mkdir -p $@


stream:
	$(MAKE) -C stream

build/libsdr_stream.a : | stream

ui:
	$(MAKE) -C ui

build/libsdr_ui.so : | ui


build/%.o : %.cpp stream/*.hpp | build
	$(CXX) $(CXXFLAGS) -o $@ $<

build/constellation.o : ui/*.hpp

build/constellation : build/constellation.o build/libsdr_stream.a build/libsdr_ui.so
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS) $(LIBS_UI)

build/signal : build/signal.o build/libsdr_stream.a
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS)

build/stream_debug : build/stream_debug.o build/libsdr_stream.a
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS)

build/throttle : build/throttle.o build/libsdr_stream.a
	$(LD) $(LDFLAGS) -o $@ $< $(LIBS)

clean:
	rm -rf build
