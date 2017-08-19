default: ../build/libsdr_stream.a

../build/stream:
	mkdir -p $@

../build/stream/stream.o : stream.cpp stream.hpp | ../build/stream
	$(CXX) $(CXXFLAGS) -o $@ $<

../build/libsdr_stream.a : ../build/stream/stream.o
	$(AR) $(ARFLAGS) $@ $^
