#ifndef FULL_H
#define	FULL_H

#include "game.h"

class Full : public Game {
        
public:
    Full();
    void search();
    
    int states;
    int nodes;
    
private:
    void traverse();
};


#endif	
