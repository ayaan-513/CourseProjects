#include "Player.h"
#include "MacUILib.h"


Player::Player(GameMechs* thisGMRef)
{
    mainGameMechsRef = thisGMRef;
    playerPosList = new objPosArrayList();
    myDir = STOP;

    objPos headPos(mainGameMechsRef -> getBoardSizeX() / 2, mainGameMechsRef -> getBoardSizeY() / 2, '*'); //initialize head of snake to middle of gameboard
    playerPosList->insertHead(headPos); //add head to the first position of the playerPosList
}


Player::~Player()
{
    // delete any heap members here
    delete playerPosList;
}

objPosArrayList* Player::getPlayerPos() const
{
    // return the reference to the playerPos arrray list
    return playerPosList;
}

void Player::updatePlayerDir()
{
        // PPA3 input processing logic  
    char input = mainGameMechsRef -> getInput(); //asynchronous input collection

    switch(input)   //FSM state transition depending on key pressed
    {
        case ' ':
                mainGameMechsRef -> setExitTrue();
                break;

        case 'w':
                if (myDir != DOWN)
                {
                    myDir = UP;
                }
                break;

        case 'a':
            if (myDir != RIGHT)
            {
                myDir = LEFT;
            }
            break;

        case 's':
            if (myDir != UP)
            {
                myDir = DOWN;
            }
            break;

        case 'd':
            if (myDir != LEFT)
            {
                myDir = RIGHT;
            }
            break;

        default:
            break;
}
}

void Player::movePlayer()
{
    // PPA3 Finite State Machine logic

    objPos temp = playerPosList -> getHeadElement(); 
    switch(myDir) //calculate the next position
    {
        
        case STOP:
        default:
            temp.pos -> x = mainGameMechsRef -> getBoardSizeX()/2;
            temp.pos -> y = mainGameMechsRef -> getBoardSizeY()/2;
            break;


        case UP:
            temp.pos -> y--;
            if (temp.pos -> y <= 0)
            {
                temp.pos -> y = mainGameMechsRef -> getBoardSizeY() - 2 ;
            }
            break;

        case DOWN:
            temp.pos -> y++;
            if (temp.pos -> y >= mainGameMechsRef -> getBoardSizeY() - 1)
            {
                temp.pos -> y = 1;
            }
            break; 
            

        case RIGHT:
            temp.pos -> x++;
            if (temp.pos -> x >= mainGameMechsRef -> getBoardSizeX() - 1)
            {
                temp.pos -> x = 1;
            }
            break;

        case LEFT:
            temp.pos -> x--;
            if (temp.pos -> x <= 0)
            {
                temp.pos -> x = mainGameMechsRef -> getBoardSizeX() - 2;
            }
            break;    
            
    }


    if (myDir != STOP) //only grow the the length of the snake/move the snake if you are not in the stop position
    {
        for (int k = 0; k < playerPosList ->getSize(); k++)
        {
            objPos reference = playerPosList -> getElement(k);
            objPos* referenceptr = &reference; //must create an additional pointer to hold the address of the playerPosList element as you cannot input the line above into the function parameter directly
            if (temp.isPosEqual(referenceptr))
            {
                mainGameMechsRef -> setExitTrue();
                mainGameMechsRef -> setLoseFlag();
                break;
                
            }
        }
        if (mainGameMechsRef -> getLoseFlagStatus() == false)
        {

        
            objPos food = (mainGameMechsRef -> getFoodPos());
            objPos *foods = &food; //get the pointer to the food objPos object
            if (!temp.isPosEqual(foods)) //check if the next calculated step of the snake is not equal to the food position (non-consumption)
            {   
                playerPosList -> insertHead(temp); //regular movement algorithm
                playerPosList -> removeTail();
            }

            else
            {
                playerPosList -> insertHead(temp); //if there is consumption, insert head but DO NOT remove tail
                mainGameMechsRef -> generateFood(playerPosList); 
                mainGameMechsRef -> incrementScore(); //generate new food and increase score
            }
        }

    }
   
        

}

// More methods to be added