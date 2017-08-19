LIBS=-ldl -lm -lGL -lglfw

default: ../build/libsdr_ui.so

../build/ui:
	mkdir -p $@

../build/ui/ui.o : ui.cpp ui.hpp font.hpp glad/*.h nanovg/*.h nuklear/*.h | ../build/ui
	$(CXX) $(CXXFLAGS) -fPIC -o $@ $<

../build/ui/glad.o : glad/glad.c glad/*.h | ../build/ui
	$(CC) $(CFLAGS_RELAXED) -fPIC -o $@ $<

../build/ui/nanovg.o : nanovg/nanovg.c nanovg/*.h | ../build/ui
	$(CC) $(CFLAGS_RELAXED) -fPIC -o $@ $<

../build/ui/nuklear.o : nuklear/nuklear.c nuklear/*.h | ../build/ui
	$(CC) $(CFLAGS_RELAXED) -fPIC -o $@ $<

../build/libsdr_ui.so : ../build/ui/glad.o ../build/ui/nanovg.o ../build/ui/nuklear.o ../build/ui/ui.o
	$(LD) $(LDFLAGS) -shared -o $@ $^ $(LIBS)
