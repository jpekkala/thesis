#CPPFLAGS = -O3 -Wextra -Wall
CPPFLAGS=-g -Wall
INC=-I ..
SOURCES=main.cpp ../retro.cpp ../retro.h ../connect4.h ../connect4.cpp

retro: $(SOURCES)
	$(CXX) $(CPPFLAGS) $(INC) -o $@ $(ENGINE_OBJ) $^

visual: $(SOURCES)
	cl /I%cd%\.. main.cpp ..\retro.cpp ..\connect4.cpp

clean:
	rm retro.exe
