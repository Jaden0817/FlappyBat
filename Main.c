#include <stdio.h>
#include "Game.h"






int main()
{

    
    Game game = {0};
    game_init(&game);

    
    game_play(&game);


    game_exit(&game);
    


}