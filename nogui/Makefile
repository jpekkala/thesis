#CPPFLAGS = -O3 -Wextra -Wall
CPPFLAGS=-g -Wall -std=c++11
INC=-I ..
ENGINE_OBJ=game.o minimax.o alphabeta.o handicap.o transtable.o connect4.o
ENGINE_SRC=$(ENGINE_OBJ:%.o=../%.cpp)

nogui: engine
	$(CXX) $(INC) $(CPPFLAGS) -o $@ $(ENGINE_OBJ) main.cpp

engine:
	$(CXX) $(INC) $(CPPFLAGS) -c $(ENGINE_SRC)

clean:
	-rm -f *.o nogui.exe nogui
