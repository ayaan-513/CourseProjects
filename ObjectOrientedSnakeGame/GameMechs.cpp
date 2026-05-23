#include "GameMechs.h"
#include "MacUILib.h"

GameMechs::GameMechs()
{
    input = 0;
    exitFlag = false;
    loseFlag = false;
    score = 0;

    boardSizeX = 20;
    boardSizeY = 10;

    food.setObjPos(-10,-10, 'o');
}

GameMechs::GameMechs(int boardX, int boardY)
{
    input = 0;
    exitFlag = false;
    loseFlag = false;
    score = 0;

    boardSizeX = boardX;
    boardSizeY = boardY;

    food.setObjPos(-10,-10, 'o');
}

GameMechs::~GameMechs() 
{
    
}
bool GameMechs::getExitFlagStatus() const
{
    return exitFlag;
}

bool GameMechs::getLoseFlagStatus() const
{
    return loseFlag;
}

char GameMechs::getInput()
{
    if(MacUILib_hasChar())
    {
        input = MacUILib_getChar();
    }
    return input;
}

void GameMechs::setExitTrue()
{
    exitFlag = true;
}

void GameMechs::setLoseFlag()
{
    loseFlag = true;
}

void GameMechs::setInput(char this_input)
{
    input = this_input;
}

void GameMechs::clearInput()
{
    input = '\0';
}

int GameMechs::getScore() const 
{
    return score;
}

void GameMechs::incrementScore() 
{
    score++;
}

int GameMechs::getBoardSizeX() const 
{
    return boardSizeX;
}

int GameMechs::getBoardSizeY() const 
{
    return boardSizeY;
}

void GameMechs::generateFood(objPosArrayList* blockOff)
{
    srand(time(NULL));

    int candidate1 = 0; //declare and initialize variables for possible x, y coordinate food location
    int candidate2 = 0;
    int count = 0; //count variable to track the generation of exactly one food coordinate




    while (count < 1)
    {
        candidate1 = rand() % (boardSizeX - 1); // Generate random x and y coordinates within the board X and Y ranges (including index 0)
        candidate2 = rand() % (boardSizeY - 1);
        for (int i = 0; i < blockOff -> getSize(); i++) //iterate through array list snake elements
        {
            count = 1;
            if ((candidate1 == blockOff -> getElement(i).pos -> x) && (candidate2 == blockOff -> getElement(i).pos -> y) || candidate1 == 0 || candidate2 == 0)
            {
                count = 0; //if the x, y candidates generated are equal to 0 or the x,y coordinates are equal to any of the positions of the snake element
                break; //set count to 0 and break so another set of candidates can be generated
            }
        }
    }

    food.pos -> x = candidate1; //set food x,y positions to the coordinates generated into candidates
    food.pos -> y = candidate2;
 

}


objPos GameMechs::getFoodPos() const
{
    return food;
}

// More methods should be added here