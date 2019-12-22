//libraries
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <LedControl.h>
#include <stdlib.h>
//defined variables

//for changing displays on the lcd
#define PLAY 0
#define OPTIONS 1
#define HIGHSCORE_MENU 2
#define INFO 3
#define MENU 4 
#define GAME_PREPARATION 5
#define GAME_PLAY 6
#define NEW_HIGHSCORE 7
#define FINAL_SCREEN 8
#define END_GAME 9

//joystick directions
#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

//options menu fields
#define CHANGE_BRIGHTNESS 0
#define CHANGE_LEVEL 1
#define BACK 2

#define BOARD_SIZE 8

//difficulty levels
#define MIN_LVL 0
#define MAX_LVL 2

//positions in boat matrix
#define TOTAL_SHIPS 0
#define TWO_LONG 1
#define THREE_LONG 2
#define FOUR_LONG 3

//orientation of boats
#define VERTICAL 1
#define HORIZONTAL 2

//player matrix field states
#define EMPTY 0
#define OCCUPIED 1//part of a boat
#define UNAVAILABLE 2//space that cannot be selected
#define HIGHLIGHTED 3//positions highlighted while placing boats

#define PLAYER_NO 2 //no of players
#define PLAYER_1 0
#define PLAYER_2 1

#define BOAT_TYPES 4

//game stages
#define PREPARATION 0
#define GAME 1

//time to display messages on lcd
#define MESSAGES_DISPLAY_TIME 1500

//blinking intervals
#define M_CURSOR_BLINK_TIME 300
#define M_UNAVAILABLE_BLINK_TIME 1000

#define MIN_BOAT_LENGTH 2
#define MAX_BOAT_LENGTH 4

//hit matrix fields states
#define MARKED 1//part of a boat but boat is not destroyed yet
#define FINAL 2//empty space or part of a destroyed boat

#define BUFFER_LENGTH 15 
//structure for coordinates in a matrix
struct Poz
{
  byte col;
  byte lin;
};
typedef struct Poz poz;

//structure for a boat
struct Boat
{
  byte length;
  poz positions[4];
};
typedef struct Boat boat;

//structure for the hit matrix
struct HitMatrixTile
{
  bool attacked;
  bool hit;
  byte state;
};
typedef struct HitMatrixTile hitTile;

//function prototypes
byte scanJoyStickMove();
void writeEEPROM(long &x);
void readEEPROM(long &x);
void clearEEPROM();
void displayInfo();
void displayMenu();
void displayHighScore();
void displayOptions();
void play();
void initializeGame();
void displayGamePreparation();
void showBoats(bool player);
void moveMatrixCursorPrep();
void checkMatrixTile();
bool checkBoat();
bool checkBuffer(int len);
void blinkTiles();
void mark(poz p);
void getBounds(poz p, int &leftMargin, int &rightMargin, int &upperMargin, int &lowerMargin);
poz findSpace();
poz getStart();
void printBoatPlace();
void printPrepMessage();
void printPositionError();
int compHoriz(const void *x, const void *y);
int compVert(const void *x, const void *y);
void displayGamePlay();
void displayTurnMessage();
void moveMatrixCursorGame();
void checkIfHit();
void blinkTiles2();
bool checkBoatDestroy(poz position);
void killBoat();
poz findSpace2();
poz getStart2();
void displayEndGame();
void displayNewHighscore();
void displayFinalScreen();

//custom character for  cursor
byte arrow[8] = {
  B10000,
  B01000,
  B00100,
  B00010,
  B00100,
  B01000,
  B10000,
};

//pins

//LCD pins
const int D4 = 8;
const int D5 = 9;
const int D6 = 10;
const int D7 = 11;
const int E = 12;
const int RS = 13;

//joystick pins
const int pinX[PLAYER_NO] = {A1, A3};
const int pinY[PLAYER_NO] = {A0, A2};
const int pinSW[PLAYER_NO] = {3, 4};

//matrix pins
const int pinDIN = 7;
const int pinLOAD = 5;
const int pinCLK = 6;
const int driverNo = 2;

//thresholds for joystick
const int highThreshold = 700;
const int lowThreshold = 400;

//check if joystick moved for each player
bool moved[PLAYER_NO] = {0, 0};
//check if joystick button is pressed for each player
bool pressed[PLAYER_NO] = {0, 0};

//current display on lcd
byte activeDisplayLCD = MENU;

//cursor positions in the main menu
poz cursorPositionsMain[4] = { {0, 0}, {7, 0}, {0, 1}, {11, 1 } };
byte cursorActualMain = 0;

//cursor positions in the options menu
poz cursorPositionsOptions[3] = { {0, 0}, {0, 1},{11,1} };
byte cursorActualOptions = 0;

String gitLink = String("https://github.com/VladSerghei/Robotics/tree/master/Matrix_Game");

int level = 0;

long score = 0;

bool playerActive = 0;//player1 is 0, player 2 is 1
byte gameStage = -1;//game phases
bool gameStarted = 0;

bool attack = 0;//to see if a player attacked during current turn

//Ships number of a player.First index is player no(0 - p1, 1 - p2), second is boat type(0 - total, 1 - two long, 2 - three long, 3 - four long)
byte shipsNo[PLAYER_NO][BOAT_TYPES];

byte maxShipsNo[BOAT_TYPES];//max ships a player can have(0 - total, 1 - two long, 2 - three long, 3 - four long)

long timer = 0;//timer for time limit
long turnTime = 0;//time taken in one turn
long turnStartTime = 0;
long timeTaken[PLAYER_NO] = {0, 0};//total time taken playing

long lastBlinkUnavailable = 0;//for blinking unavailable matrix LEDs
int unavailableState = 0;

byte boatLen = 0;//variables used at placing or destroying boats
byte count = 0;


//currently selected space in matrix, first is for P1, second for P2
poz cursorMatrix[PLAYER_NO] = {{0, 0}, {0, 0} };

bool matrixCursorState[PLAYER_NO] = {0, 0};//for blinking the cursor
long lastMatrixCursorBlink = 0;

poz pozBuffer[4];//buffer for loading the boats
poz markedBuffer[PLAYER_NO][BUFFER_LENGTH];
byte markBufferIndex[PLAYER_NO] = {0, 0};

//initialize lcd and matrix
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
LedControl matrix = LedControl(pinDIN, pinCLK, pinLOAD, driverNo); 

byte boatMatrix[PLAYER_NO][BOARD_SIZE][BOARD_SIZE];
hitTile hitMatrix[PLAYER_NO][BOARD_SIZE][BOARD_SIZE];

boat playerBoats[PLAYER_NO][5];//boats owned by a player, a player has 5 boats on any difficulty

int hit = 1;//used to see if a player has hit something when he attacked

long lastBlinkMarked = 0;//for blinking marked spaces
int markedState = 0;

long infoStart = 0;//time variables for info screen
long infoTime = 0;

byte infoState = 0;//this determines what is printed in the upper part of the info screen

int infoIndex = 0;//index of scrolling the info screen
int brightness = 7;//matrix brightness

//program
void setup()
{
  lcd.createChar(0, arrow);
  lcd.begin(16, 2);
  long msgTime = millis();
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("BOOTING GAME");
  lcd.setCursor(2, 1);
  lcd.print("PLEASE WAIT!");

  for (int i = 0; i < driverNo; i++)
  {
    matrix.setIntensity(i, brightness); // set matrix brightness
    matrix.clearDisplay(i);
  }

  for (int i = 0; i < PLAYER_NO; i++)
  {
    pinMode(pinX[i], INPUT);
    pinMode(pinY[i], INPUT);
    pinMode(pinSW[i], INPUT_PULLUP);
  }
  

  Serial.begin(9600);
  
  while (millis() - msgTime < 1000);

  lcd.clear();
}

void loop()
{
  
  switch (activeDisplayLCD)
  {
  case MENU:
    displayMenu();
    break;

  case PLAY:
    play();
    break;

  case END_GAME:
    displayEndGame();
    break;

  case OPTIONS:
    displayOptions();
    break;

  case HIGHSCORE_MENU:
    displayHighScore();
    break;

  case GAME_PREPARATION:
    displayGamePreparation();
    break;

  case GAME_PLAY:
    displayGamePlay();
    break;

  case NEW_HIGHSCORE:
    displayNewHighscore();
    break;
  case FINAL_SCREEN:
    displayFinalScreen();
    break;
  case INFO:
    displayInfo();
    break;
  }









}
//functions

//see where joystick is moved
byte scanJoyStickMove()
{
  int xValue = analogRead(pinX[playerActive]);
  int yValue = analogRead(pinY[playerActive]);
  if (xValue > highThreshold && moved[playerActive] == false)
  {
    moved[playerActive] = true;
    return RIGHT;
  }
  if (xValue < lowThreshold && moved[playerActive] == false)
  {
    moved[playerActive] = true;
    return LEFT;
  }
  if (yValue > highThreshold && moved[playerActive] == false)
  {
    moved[playerActive] = true;
    return DOWN;
  }
  if (yValue < lowThreshold && moved[playerActive] == false)
  {
    moved[playerActive] = true;
    return UP;
  }
  if ((xValue > lowThreshold) && (xValue < highThreshold) && (yValue > lowThreshold) && (yValue < highThreshold))
    moved[playerActive] = false;
  return 0;
}

//write x to eeprom
void writeEEPROM(long &x)
{
  byte *v = (byte*)(void*)&x;
  int addr = 0;
  for (int i = 0; i < sizeof(x); i++)
    EEPROM.write(addr++, *v++);

}

//read value from eeprom into x
void readEEPROM(long &x)
{
  byte *v = (byte*)(void*)&x;
  int addr = 0;
  for (int i = 0; i < sizeof(x); i++)
    *v++ = EEPROM.read(addr++);

}

//clear eeprom
void clearEEPROM()
{
  for (int i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, 0);
  }
}

//display info screen
void displayInfo()
{
  if (infoStart == 0)//when opening info screen
  {
    infoStart = millis();
    infoTime = infoStart;
    infoIndex = 0;
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++)
      lcd.print(gitLink[i]);
  }
  switch(infoState)
  {
  case 0:
    lcd.setCursor(0, 0);
    lcd.print("BATTLESHIPS");
    break;
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("VLAD SERGHEI");
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print("@UNIBUC ROBOTICS");
    break;
  }

  if (millis() - infoTime > 1500)//change what is printed on the upper row
  {
    if (infoState == 2)
      infoState = 0;
    else
      infoState += 1;
    
    infoTime = millis();
    lcd.clear();
  }

  if (millis() - infoStart > 500)//scroll the github link
  {

    infoIndex += 1;
    infoStart = millis();
    lcd.setCursor(0, 1);
    for (int i = infoIndex; i < infoIndex + 16; i++)
      lcd.print(gitLink[i]);
  }
  if (infoIndex == strlen("https://github.com/VladSerghei/Robotics/tree/master/Matrix_Game") - 16)
  {
    infoIndex = 0;
    lcd.setCursor(0, 1);
    lcd.print("https://github.com/VladSerghei/Robotics/tree/master/Matrix_Game");
  }

  int valSW = !digitalRead(pinSW[playerActive]);
  if (valSW == 1 && pressed[playerActive] == 0)//check if joystick is pressed
  {
    lcd.clear();
    activeDisplayLCD = MENU;//go back to menu
    pressed[playerActive] = 1;
    infoIndex = 0;
    return;
  }
  if (valSW == 0 && pressed[PLAYER_1] == 1)
  {
    pressed[playerActive] = 0;
  }
}

//displaying the main menu
void displayMenu()
{

  lcd.setCursor(1, 0);
  lcd.print("PLAY");
  lcd.setCursor(8, 0);
  lcd.print("SETTINGS");
  lcd.setCursor(1, 1);
  lcd.print("HIGHSCORE");
  lcd.setCursor(12, 1);
  lcd.print("INFO");
  
  lcd.setCursor(cursorPositionsMain[cursorActualMain].col, cursorPositionsMain[cursorActualMain].lin);
  lcd.write(byte(0));



  //change menu selection
  byte cursorMove = scanJoyStickMove();
  if (cursorMove == LEFT)
  {
    if (cursorActualMain == 0)
      cursorActualMain = 3;
    else
      cursorActualMain = cursorActualMain - 1;
    lcd.clear();
  }
  else if (cursorMove == RIGHT)
  {
    if (cursorActualMain == 3)
      cursorActualMain = 0;
    else
      cursorActualMain = cursorActualMain + 1;
    lcd.clear();
  }

  int valSW = !digitalRead(pinSW[PLAYER_1]);
  if (valSW == 1 && pressed[PLAYER_1] == 0)//check if joystick is pressed
  {
    lcd.clear();
    activeDisplayLCD = cursorActualMain;//change display
    pressed[PLAYER_1] = 1;
    return;
  }
  if (valSW == 0 && pressed[PLAYER_1] == 1)
  {
    pressed[PLAYER_1] = 0;
  }

}

//displaying the highscore menu
void displayHighScore()
{
  long highscore;
  readEEPROM(highscore);
  
  lcd.setCursor(0, 0);
  lcd.print("HIGHSCORE:");
  lcd.print(highscore);

  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.print("EXIT");

  int SWvalue = !digitalRead(pinSW[PLAYER_1]);
  if (SWvalue == 1 && pressed[PLAYER_1] == 0)//check if button pressed
  {
    activeDisplayLCD = MENU;//go back to main menu
    pressed[PLAYER_1] = 1;
    lcd.clear();
    return;
  }
  if (SWvalue == 0 && pressed[PLAYER_1] == 1)
  {
    pressed[PLAYER_1] = 0;
  }
}

//displaying the options menu
void displayOptions()
{
  lcd.setCursor(1, 0);//print level
  lcd.print("BRIGHTNESS:");
  lcd.print(brightness);
  
  lcd.setCursor(1, 1);//print level
  lcd.print("LEVEL:");
  lcd.print(level);

  lcd.setCursor(12, 1);//print exit option
  lcd.print("EXIT");

  //print cursor
  lcd.setCursor(cursorPositionsOptions[cursorActualOptions].col, cursorPositionsOptions[cursorActualOptions].lin);
  lcd.write(byte(0));

  //change selected field
  int cursorMove = scanJoyStickMove();
  if (cursorMove == LEFT)
  {
    if (cursorActualOptions == 0)
      cursorActualOptions = 2;
    else 
      cursorActualOptions -= 1;
    lcd.clear();
  }
  if (cursorMove == RIGHT)
  {
    if (cursorActualOptions == 2)
      cursorActualOptions = 0;
    else
      cursorActualOptions += 1;
    lcd.clear();
  }
  //if current field is exit and it is selceted, go back to main menu
  int SWvalue = !digitalRead(pinSW[PLAYER_1]);
  if (SWvalue == 1 && pressed[PLAYER_1] == 0)
  {
    pressed[PLAYER_1] = 1;
    if (cursorActualOptions == BACK)
    {
      activeDisplayLCD = MENU;
      cursorActualOptions = 0;
      lcd.clear();
      for (int i = 0; i < driverNo; i++)
        matrix.setIntensity(i, brightness);
      return;
    }
  }
  if (SWvalue == 0 && pressed[PLAYER_1] == 1)
  {
    pressed[PLAYER_1] = 0;
  }

  //change level if selected
  if (cursorActualOptions == CHANGE_LEVEL)
  {
    if (cursorMove == UP)
    {
      if (level == MAX_LVL)
        level = MIN_LVL;
      else
        level = level + 1;
      lcd.clear();
    }
    else if (cursorMove == DOWN)
    {
      if (level == MIN_LVL)
        level = MAX_LVL;
      else
        level = level - 1;
      lcd.clear();
    }
  }
  //change brightness if selected
  if (cursorActualOptions == CHANGE_BRIGHTNESS)
  {
    if (cursorMove == UP)
    {
      if (brightness == 15)
        brightness = 0;
      else
        brightness = brightness + 1;
      lcd.clear();
    }
    else if (cursorMove == DOWN)
    {
      if (brightness == 0)
        brightness = 15;
      else
        brightness = brightness - 1;
      lcd.clear();
    }
  }


}

//displaying the game
void play()
{
  if (gameStarted == false)//when game starts
  {
    lcd.clear();//clearing lcd

    long msgTime = millis();

    lcd.setCursor(3, 0);//display starting mesage
    lcd.print("BEGINNING");
    lcd.setCursor(2, 1);
    lcd.print("PREPARATION");

    initializeGame();

    while (millis() - msgTime < MESSAGES_DISPLAY_TIME);//wait for the message to be displayed for enough time
    
    gameStarted = true;//mark that game has started
  }
  switch (gameStage)
  {
  case PREPARATION:
    activeDisplayLCD = GAME_PREPARATION;
    matrix.shutdown(PLAYER_1, false);//power up matrixes
    matrix.shutdown(PLAYER_2, false);

    lcd.clear();
    return;
  case GAME:
    activeDisplayLCD = GAME_PLAY;
    //reset needed variables
    cursorMatrix[PLAYER_1] = {0, 0};
    cursorMatrix[PLAYER_2] = {0, 0};
    
    attack = 0;
    timeTaken[PLAYER_1] = 0;
    timeTaken[PLAYER_2] = 0;

    lcd.clear();
    
    turnStartTime = millis();
    return;

  }


}

//initialize game variables
void initializeGame()
{
  maxShipsNo[TOTAL_SHIPS] = 5;//number of total ships

  maxShipsNo[TWO_LONG] = level + 2;//number of ships that occupy 2 spaces

  if (level == 0)
  {
    maxShipsNo[THREE_LONG] = 2;//number of ships that occupy 3 spaces
    timer = 120;//turns timer(in seconds)
  }
  else
  {
    maxShipsNo[THREE_LONG] = 1;//number of ships that occupy 3 spaces
    timer = 30 - 10 * level;
  }
  timer *= 1000;
  if (level == 2)//number of ships that occupy 4 spaces
    maxShipsNo[FOUR_LONG] = 0;
  else
    maxShipsNo[FOUR_LONG] = 1;

  for (int i = 0; i < 4; i++)//clear ship number of players
  {
    shipsNo[PLAYER_1][i] = 0;
    shipsNo[PLAYER_2][i] = 0;
  }

  for (int i = 0; i < BOARD_SIZE; i++)//clear ships matrix
    for (int j = 0; j < BOARD_SIZE; j++)
    {
      boatMatrix[PLAYER_1][i][j] = 0;
      boatMatrix[PLAYER_2][i][j] = 0;
      hitMatrix[PLAYER_1][i][j].attacked = 0;
      hitMatrix[PLAYER_1][i][j].hit = 0;
      hitMatrix[PLAYER_1][i][j].state = 0;
      hitMatrix[PLAYER_2][i][j].attacked = 0;
      hitMatrix[PLAYER_2][i][j].hit = 0;
      hitMatrix[PLAYER_2][i][j].state = 0;
    }

  cursorMatrix[PLAYER_1] = { 0, 0 };//initialize player matrix cursors
  cursorMatrix[PLAYER_2] = { 0, 0 };

  count = 0;//clear counter variable

  score = 0;
  gameStage = PREPARATION;

  matrix.shutdown(PLAYER_1, false);//start matrix
  playerActive = PLAYER_1;

  boatLen = MIN_BOAT_LENGTH;

}

//display the game preparation phase, where users place ships
void displayGamePreparation()
{
  printPrepMessage();

  for (byte j = 0; j < maxShipsNo[boatLen - 1]; j++)//no of boats of length boatLen is at position (boatLen - 1)
  {
    while (count < boatLen)
    {
      matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);//show cursor
      
      moveMatrixCursorPrep();//check if it is moved
      
      checkMatrixTile();//check if a space is selected and if so do the appropriate actions
      
      blinkTiles();//change cursor state at a certain amount of time
    }

    matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, false);//turn off cursor temporarily

    if (!checkBoat())//if boat is placed incorrectly
      j -= 1;
    
    cursorMatrix[playerActive] = getStart();

  }

  boatLen += 1;

  if (boatLen > MAX_BOAT_LENGTH)//at the end of a player's turn
  {
    matrix.clearDisplay(playerActive);
    showBoats(playerActive);//display placed boats
    lcd.clear();
    
    if (playerActive == PLAYER_1)//get to player 2
    {
      
      boatLen = MIN_BOAT_LENGTH;    
      
      lastBlinkUnavailable = 0;
      lastMatrixCursorBlink = 0;
    }
    else//start game
    {
      activeDisplayLCD = PLAY;
      gameStage += 1;
      lcd.setCursor(0, 0);
      lcd.print("ALL BOATS PLACED");
      lcd.setCursor(1, 1);
      lcd.print("BEGINNING GAME");
      delay(3000);//gives time for players to see the message and their placed boats
      lcd.clear();
      matrix.clearDisplay(PLAYER_1);
      matrix.clearDisplay(PLAYER_2);
    }
    playerActive = !playerActive;

  }
  
}

//show boats placed at the end of preparation phase
void showBoats(bool player)
{
  for (byte i = 0; i < BOARD_SIZE; i++)
    for (byte j = 0; j < BOARD_SIZE; j++)
      if (boatMatrix[player][i][j] == OCCUPIED)
        matrix.setLed(player, j, i, true);
}

//move matrix cursor during preparation phase
void moveMatrixCursorPrep()
{
  int cursorMove = scanJoyStickMove();
  if (cursorMove != 0)//check if joystick was moved
  {
    //turn off actual cursor position
    matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, false);

    //light the space from where the cursor is moved according to its state
    if (boatMatrix[playerActive][cursorMatrix[playerActive].lin][cursorMatrix[playerActive].col] == HIGHLIGHTED)
      matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, true);
    else if (boatMatrix[playerActive][cursorMatrix[playerActive].lin][cursorMatrix[playerActive].col] == OCCUPIED)
      matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, true);
    else if (boatMatrix[playerActive][cursorMatrix[playerActive].lin][cursorMatrix[playerActive].col] == UNAVAILABLE)
      matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, unavailableState);

    switch (cursorMove)//get direction of joystick movement
    {
    case UP:
      if (cursorMatrix[playerActive].lin == 0)
        cursorMatrix[playerActive].lin = BOARD_SIZE - 1;
      else
        cursorMatrix[playerActive].lin -= 1;
      break;

    case DOWN:
      if (cursorMatrix[playerActive].lin == BOARD_SIZE - 1)
        cursorMatrix[playerActive].lin = 0;
      else
        cursorMatrix[playerActive].lin += 1;
      break;

    case LEFT:
      if (cursorMatrix[playerActive].col == 0)
        cursorMatrix[playerActive].col = BOARD_SIZE - 1;
      else
        cursorMatrix[playerActive].col -= 1;
      break;

    case RIGHT:
      if (cursorMatrix[playerActive].col == BOARD_SIZE - 1)
        cursorMatrix[playerActive].col = 0;
      else
        cursorMatrix[playerActive].col += 1;
      break;
    }
  }
  //turn on new position
  matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);

}

//check if a matrix tile can be selected and if so add it to the buffer
void checkMatrixTile()
{
  int SWvalue = !digitalRead(pinSW[playerActive]);//check if joystick button is pressed
  if (SWvalue == 1 && pressed[playerActive] == 0)
  {
    pressed[playerActive] = 1;


    //check if selected space is not unavailable
    if (boatMatrix[playerActive][cursorMatrix[playerActive].lin][cursorMatrix[playerActive].col] == EMPTY)
    {

      pozBuffer[count] = cursorMatrix[playerActive];//add selected position to buffer
      //change state of selected space in matrix
      boatMatrix[playerActive][pozBuffer[count].lin][pozBuffer[count].col] = HIGHLIGHTED;
      matrix.setLed(playerActive, pozBuffer[count].col, pozBuffer[count].lin, true);

      count += 1;
      //temporary selected space is lit

      poz newPoz = findSpace();//move cursor from selected space
      cursorMatrix[playerActive] = newPoz;

      //light up cursor
      matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);
    }
  }

  if (SWvalue == 0 && pressed[playerActive] == 1)
  {
    pressed[playerActive] = 0;
  }
}

//check if a boat was placed
bool checkBoat()
{
  if (!checkBuffer(count))//check if the positions in the buffer can form a boat
  {
    printPositionError();//print error message on lcd if incorrect

    for (int i = 0; i < count; i++)
    {
      boatMatrix[playerActive][pozBuffer[i].lin][pozBuffer[i].col] = EMPTY;//unmark used spaces
      matrix.setLed(playerActive, pozBuffer[i].col, pozBuffer[i].lin, false);//turn off coresponding LEDs
    }
    count = 0;
    return 0;
  }
  else//if positioning is good
  {
    for (int k = 0; k < count; k++)//mark boat on map
    {
      mark(pozBuffer[k]);
      playerBoats[playerActive][shipsNo[playerActive][TOTAL_SHIPS]].positions[k] = pozBuffer[k];
      playerBoats[playerActive][shipsNo[playerActive][TOTAL_SHIPS]].length = count;
    }

    shipsNo[playerActive][boatLen - 1] += 1;//update number of owned ships
    shipsNo[playerActive][TOTAL_SHIPS] += 1;

    printBoatPlace();//print message on lcd to say that boat is placed
    printPrepMessage();

    count = 0;//reset count
    return 1;
  }
}

//check if buffer positions can form a ship
bool checkBuffer(int len)
{
  int orientation = 0;
  if (pozBuffer[1].lin - pozBuffer[0].lin != 0)
    orientation = VERTICAL;
  else
    orientation = HORIZONTAL;
  switch (orientation)
  {
  case VERTICAL:
    qsort(pozBuffer, len, sizeof(poz), compVert);
    for (int i = 0; i < len - 1; i++)
    {
      if (pozBuffer[i].lin - pozBuffer[i + 1].lin == 0 || pozBuffer[i].col - pozBuffer[i + 1].col != 0)
        return 0;//check if on same column
      if (pozBuffer[i].lin - pozBuffer[i + 1].lin > 1 || pozBuffer[i].lin - pozBuffer[i + 1].lin < -1)
        return 0;//check if it doesn't go past matrix edges
    }
    break;
  case HORIZONTAL:
    qsort(pozBuffer, len, sizeof(poz), compHoriz);
    for (int i = 0; i < len - 1; i++)
    {
      if (pozBuffer[i].lin - pozBuffer[i + 1].lin != 0 || pozBuffer[i].col - pozBuffer[i + 1].col == 0)
        return 0;//check if on same row
      if (pozBuffer[i].col - pozBuffer[i + 1].col > 1 || pozBuffer[i].col - pozBuffer[i + 1].col < -1)
        return 0;//check if it doesn't go past matrix edges
    }
    break;
  }
  return 1;
}

//blink tiles during preparation phase
void blinkTiles()
{
  if (millis() - lastBlinkUnavailable > M_UNAVAILABLE_BLINK_TIME)
  {
    unavailableState = !unavailableState;
    lastBlinkUnavailable = millis();
    for (int i = 0; i < shipsNo[playerActive][TOTAL_SHIPS]; i++)
    {
      
        int leftMargin, rightMargin, upperMargin, lowerMargin, util;
        getBounds(playerBoats[playerActive][i].positions[0], leftMargin, util, upperMargin, util);
        getBounds(playerBoats[playerActive][i].positions[playerBoats[playerActive][i].length - 1], util, rightMargin, util, lowerMargin);

        for (int j = leftMargin; j <= rightMargin; j++)
        {
          if (boatMatrix[playerActive][upperMargin][j] == UNAVAILABLE)
            if (upperMargin != cursorMatrix[playerActive].lin || j != cursorMatrix[playerActive].col)
              matrix.setLed(playerActive, j, upperMargin, unavailableState);

          if (boatMatrix[playerActive][lowerMargin][j] == UNAVAILABLE)
            if (lowerMargin != cursorMatrix[playerActive].lin || j != cursorMatrix[playerActive].col)
              matrix.setLed(playerActive, j, lowerMargin, unavailableState);
        }

        for (int j = upperMargin; j <= lowerMargin; j++)
        {
          if (boatMatrix[playerActive][j][leftMargin] == UNAVAILABLE)
            if (j != cursorMatrix[playerActive].lin || leftMargin != cursorMatrix[playerActive].col)
              matrix.setLed(playerActive, leftMargin, j, unavailableState);

          if (boatMatrix[playerActive][j][rightMargin] == UNAVAILABLE)
            if (j != cursorMatrix[playerActive].lin || rightMargin != cursorMatrix[playerActive].col)
              matrix.setLed(playerActive, rightMargin, j, unavailableState);
        }
        
    }
  }
  if (millis() - lastMatrixCursorBlink > M_CURSOR_BLINK_TIME)
  {
    lastMatrixCursorBlink = millis();
    matrixCursorState[playerActive] = !matrixCursorState[playerActive];
    matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);
  }


}

//mark a boat that is placed and the positions around it
void mark(poz p)
{
  int leftMargin, rightMargin, upperMargin, lowerMargin;
  getBounds(p, leftMargin, rightMargin, upperMargin, lowerMargin);

  for (int i = upperMargin; i <= lowerMargin; i++)
    for (int j = leftMargin; j <= rightMargin; j++)
      if (boatMatrix[playerActive][i][j] == EMPTY)
      {
        boatMatrix[playerActive][i][j] = UNAVAILABLE;
        matrix.setLed(playerActive, j, i, unavailableState);
      }

  boatMatrix[playerActive][p.lin][p.col] = OCCUPIED;
  matrix.setLed(playerActive, p.col, p.lin, true);
}

//get bounds of a position
void getBounds(poz p, int &leftMargin, int &rightMargin, int &upperMargin, int &lowerMargin)
{
  if (p.lin - 1 < 0)
    upperMargin = 0;
  else
    upperMargin = p.lin - 1;

  if (p.lin + 1 > 7)
    lowerMargin = 7;
  else
    lowerMargin = p.lin + 1;

  if (p.col - 1 < 0)
    leftMargin = 0;
  else
    leftMargin = p.col - 1;

  if (p.col + 1 > 7)
    rightMargin = 7;
  else
    rightMargin = p.col + 1;
}

//find an empty space around the current position
poz findSpace()
{
  
  int leftMargin, rightMargin, upperMargin, lowerMargin;
  getBounds(cursorMatrix[playerActive], leftMargin, rightMargin, upperMargin, lowerMargin);
  poz p;
  

  for (int i = upperMargin; i <= lowerMargin; i++)
    for (int j = leftMargin; j <= rightMargin; j++)
    {
      if (boatMatrix[playerActive][i][j] == EMPTY)
      {
        p.lin = i;
        p.col = j;
        return p;
      }
    }
  return getStart();

}

//find an empty space in the whole matrix
poz getStart()
{
  poz p;
  for (int i = 0; i < BOARD_SIZE; i++)
    for (int j = 0; j < BOARD_SIZE; j++)
      if (boatMatrix[playerActive][i][j] == 0)
      {
        p.lin = i;
        p.col = j;
        return p;
      }

  return p;
}

//print boat placed message
void printBoatPlace()
{
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("BOAT");
  lcd.setCursor(5, 1);
  lcd.print("PLACED");
  delay(1500);
  printPrepMessage();
}

//print preparation message
void printPrepMessage()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PLAYER ");
  lcd.print(playerActive + 1);
  lcd.setCursor(0, 1);
  lcd.print("PLACE ");
  lcd.print(maxShipsNo[boatLen - 1] - shipsNo[playerActive][boatLen - 1]);
  lcd.print(" ");
  lcd.print(boatLen);
  lcd.print("-LONG");
}

//print positioning error
void printPositionError()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOAT POSITIONING");
  lcd.setCursor(4, 1);
  lcd.print("INCORRECT");
  delay(1500);
  printPrepMessage();
}

//functions for qsort to sort a boat placed horizontally(left to right) or vertically(upper to lower)
int compHoriz(const void *x, const void *y)
{
  poz a = *((poz *)x);
  poz b = *((poz *)y);
  return a.col - b.col;
}
int compVert(const void *x, const void *y)
{
  
  poz a = *((poz *)x);
  poz b = *((poz *)y);
  
  return a.lin - b.lin;
  
}

//playing stage
void displayGamePlay()
{
  turnTime = millis() - turnStartTime;
  displayTurnMessage();
  hit = 1;

  matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);//show cursor

  moveMatrixCursorGame();//check if joystick is moved and if so move the cursor
  checkIfHit();//check if the button is pressed, if the selected space can be attacked and if it's a hit or a miss

  blinkTiles2();//blink matrix tiles
  

  if (shipsNo[!playerActive][TOTAL_SHIPS] == 0)//endgame condition
  {
    activeDisplayLCD = END_GAME;
    lcd.clear();
    

  }
  if (hit == 0 || turnTime > timer)//next players turn if miss or time ended
  {
    timeTaken[playerActive] += turnTime;
    
    if (hitMatrix[playerActive][cursorMatrix[playerActive].lin][cursorMatrix[playerActive].col].attacked == 0)
      matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, false);
    if (hit == 0)
    {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("YOU MISSED!");
      delay(1000);
      lcd.clear();
    }
    else
    {
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("TIME OVER!");
      delay(1000);
      lcd.clear();
    }

    playerActive = !playerActive;//change player

    turnStartTime = millis();//reset turn start time
  }
  else if (attack == 1)//if hit shoot again
  {
    timeTaken[playerActive] += turnTime;
    attack = 0;
    
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("IT'S A HIT!");
    delay(1000);
    lcd.clear();
    
    turnStartTime = millis();
  }

}

//display whose turn it is, time left and wha they have left to hit
void displayTurnMessage()
{
//  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PLAYER");
  lcd.print(playerActive + 1);
  lcd.print(" ");
  
  lcd.print("TIME:   ");
  lcd.setCursor(13, 0);
  lcd.print((timer - turnTime) / 1000);

  lcd.setCursor(0, 1);
  lcd.print("BOAT:");

  
  lcd.print(TWO_LONG + 1);
  lcd.print("-");
  lcd.print(shipsNo[!playerActive][TWO_LONG]);
  lcd.print(" ");
  
  lcd.print(THREE_LONG + 1);
  lcd.print("-");
  lcd.print(shipsNo[!playerActive][THREE_LONG]);
  lcd.print(" ");

  lcd.print(FOUR_LONG + 1);
  lcd.print("-");
  lcd.print(shipsNo[!playerActive][FOUR_LONG]);
  

}

//move matrix cursor during game phase
void moveMatrixCursorGame()
{
  int cursorMove = scanJoyStickMove();
  if (cursorMove != 0)//check if joystick was moved
  {
    //turn off actual cursor position
    matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, false);
    poz position = cursorMatrix[playerActive];
    if (hitMatrix[playerActive][position.lin][position.col].attacked == 1)//light the position from where the cursor is moved according to its state
    {
      if (hitMatrix[playerActive][position.lin][position.col].hit == 1)
      {
        if (hitMatrix[playerActive][position.lin][position.col].state == FINAL)
          matrix.setLed(playerActive, position.col, position.lin, true);
        else
          matrix.setLed(playerActive, position.col, position.lin, markedState);
      }
      else
        matrix.setLed(playerActive, position.col, position.lin, true);
    }



    switch (cursorMove)//get direction of joystick movement
    {
    case UP:
      if (cursorMatrix[playerActive].lin == 0)
        cursorMatrix[playerActive].lin = BOARD_SIZE - 1;
      else
        cursorMatrix[playerActive].lin -= 1;
      break;

    case DOWN:
      if (cursorMatrix[playerActive].lin == BOARD_SIZE - 1)
        cursorMatrix[playerActive].lin = 0;
      else
        cursorMatrix[playerActive].lin += 1;
      break;

    case LEFT:
      if (cursorMatrix[playerActive].col == 0)
        cursorMatrix[playerActive].col = BOARD_SIZE - 1;
      else
        cursorMatrix[playerActive].col -= 1;
      break;

    case RIGHT:
      if (cursorMatrix[playerActive].col == BOARD_SIZE - 1)
        cursorMatrix[playerActive].col = 0;
      else
        cursorMatrix[playerActive].col += 1;
      break;
    }
    matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);//turn on new position
  }
  
  

}

//check if joystick button was pressed, if the space has not been already attacked and if it is a hit or a miss
void checkIfHit()
{
  int SWvalue = !digitalRead(pinSW[playerActive]);//check if joystick button is pressed
  if (SWvalue == 1 && pressed[playerActive] == 0)
  {
    pressed[playerActive] = 1;

    poz position = cursorMatrix[playerActive];

    
    if (hitMatrix[playerActive][position.lin][position.col].attacked == 0)//if space was not attacked yet
      if (boatMatrix[!playerActive][position.lin][position.col] == OCCUPIED)//if it is a hit
      {
        matrix.setLed(playerActive, position.col, position.lin, false);

        hitMatrix[playerActive][position.lin][position.col].attacked = 1;
        hitMatrix[playerActive][position.lin][position.col].hit = 1;
        hitMatrix[playerActive][position.lin][position.col].state = MARKED;

        markedBuffer[playerActive][markBufferIndex[playerActive]] = position;
        markBufferIndex[playerActive] += 1;
        //Serial.println(markBufferIndex[playerActive]);
        matrix.setLed(playerActive, position.col, position.lin, markedState);

        cursorMatrix[playerActive] = findSpace2();

        //light up cursor
        matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);

        count = 0;
        if (checkBoatDestroy(position) == 1)
          killBoat();

        attack = 1;

      }
      else//if missed
      {
        matrix.setLed(playerActive, position.col, position.lin, false);

        hitMatrix[playerActive][position.lin][position.col].attacked = 1;
        hitMatrix[playerActive][position.lin][position.col].hit = 0;
        hitMatrix[playerActive][position.lin][position.col].state = FINAL;

        matrix.setLed(playerActive, position.col, position.lin, true);

        cursorMatrix[playerActive] = findSpace2();

        matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);
        hit = 0;
      }
  }

  if (SWvalue == 0 && pressed[playerActive] == 1)
  {
    pressed[playerActive] = 0;
  }
}

//blink matrix tiles during game phase
void blinkTiles2()
{
  
  if (millis() - lastBlinkMarked > M_UNAVAILABLE_BLINK_TIME)
  {
    markedState = !markedState;
    lastBlinkMarked = millis();
    for (int i = 0; i < markBufferIndex[playerActive]; i++)
    {
      poz p = markedBuffer[playerActive][i];

      if (p.lin != cursorMatrix[playerActive].lin || p.col != cursorMatrix[playerActive].col)
        matrix.setLed(playerActive, p.col, p.lin, markedState);
    }
    for (int i = 0; i < markBufferIndex[!playerActive]; i++)
    {
      poz p = markedBuffer[!playerActive][i];
  
      matrix.setLed(!playerActive, p.col, p.lin, markedState);
    }

  }
  if (millis() - lastMatrixCursorBlink > M_CURSOR_BLINK_TIME)
  {
    lastMatrixCursorBlink = millis();
    matrixCursorState[playerActive] = !matrixCursorState[playerActive];
    matrix.setLed(playerActive, cursorMatrix[playerActive].col, cursorMatrix[playerActive].lin, matrixCursorState[playerActive]);
  }
}

//check if the boat containing position is destroyed
bool checkBoatDestroy(poz position)
{
  int leftMargin, rightMargin, upperMargin, lowerMargin;
  getBounds(position, leftMargin, rightMargin, upperMargin, lowerMargin);

  int ok = 1;
  for (int k = 0; k < count; k++)
    if (pozBuffer[k].lin == position.lin && pozBuffer[k].col == position.col)
      ok = 0;
  if (ok == 1)
    pozBuffer[count++] = position;
  else
    return 0;
  int retVal = 1;
  for (byte i = upperMargin; i <= lowerMargin; i++)
    for (byte j = leftMargin; j <= rightMargin; j++)
    {
      if (boatMatrix[!playerActive][i][j] == OCCUPIED)
        if (hitMatrix[playerActive][i][j].attacked == 1)
        {
          poz p = { j, i };
          ok = 1;
          for (int k = 0; k < count; k++)
            if (pozBuffer[k].lin == p.lin && pozBuffer[k].col == p.col)
            {
              ok = 0;
              break;
            }
          if (ok == 1)
            retVal = min(checkBoatDestroy(p), retVal);

        }
        else
          return 0;
    }
  return retVal;
}

//remove destroyed boat
void killBoat()
{
  long msgTime = millis();
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Boat destroyed");
  lcd.setCursor(5, 1);
  lcd.print(count);
  lcd.print("-long");

  if (pozBuffer[1].lin - pozBuffer[0].lin != 0)
    qsort(pozBuffer, count, sizeof(poz), compVert);
  else
    qsort(pozBuffer, count, sizeof(poz), compHoriz);

  int leftMargin, rightMargin, upperMargin, lowerMargin, util;
  getBounds(pozBuffer[0], leftMargin, util, upperMargin, util);
  getBounds(pozBuffer[count - 1], util, rightMargin, util, lowerMargin);

  for (int i = upperMargin; i <= lowerMargin; i++)
    for (int j = leftMargin; j <= rightMargin; j++)
    {
      hitMatrix[playerActive][i][j].attacked = 1;
      hitMatrix[playerActive][i][j].hit = 1;
      hitMatrix[playerActive][i][j].state = FINAL;
      matrix.setLed(playerActive, j, i, true);
    }
  for (int i = 0; i < count; i++)
  {
    for(int j = 0; j < markBufferIndex[playerActive]; j ++)
      if (pozBuffer[i].lin == markedBuffer[playerActive][j].lin && pozBuffer[i].col == markedBuffer[playerActive][j].col)
      {
        markedBuffer[playerActive][j] = markedBuffer[playerActive][markBufferIndex[playerActive] - 1];
        markBufferIndex[playerActive] -= 1;
      }

  }
  cursorMatrix[playerActive] = getStart2();
  shipsNo[!playerActive][TOTAL_SHIPS] -= 1;
  shipsNo[!playerActive][count - 1] -= 1;
  
  count = 0;

  while (millis() - msgTime < MESSAGES_DISPLAY_TIME);

  lcd.clear();


}

//find a space not attacked around the cursor
poz findSpace2()
{
  int leftMargin, rightMargin, upperMargin, lowerMargin;
  getBounds(cursorMatrix[playerActive], leftMargin, rightMargin, upperMargin, lowerMargin);
  poz p;
  

  for (int i = upperMargin; i <= lowerMargin; i++)
    for (int j = leftMargin; j <= rightMargin; j++)
    {
      if (hitMatrix[playerActive][i][j].attacked == 0)
      {
        p.lin = i;
        p.col = j;
        return p;
      }
    }
  return getStart2();
}

//find a space not attacked in the whole matrix
poz getStart2()
{
  poz p;
  for (int i = 0; i < BOARD_SIZE; i++)
    for (int j = 0; j < BOARD_SIZE; j++)
      if (hitMatrix[playerActive][i][j].attacked == 0)
      {
        p.lin = i;
        p.col = j;
        return p;
      }

  
}

// displaying the endgame screen
void displayEndGame()
{
  long msgStartTime = millis();

  timeTaken[playerActive] /= 1000;//convert to seconds

  score = (1.0 / timeTaken[playerActive])*(level + 1.0) * 100000.0;//10000 not to have score under 1

  lcd.setCursor(0, 0);
  lcd.print("PLAYER ");
  lcd.print(playerActive + 1);
  lcd.print(" HAS WON");

  lcd.setCursor(0, 1);
  lcd.print("SCORE:");
  lcd.print(score);

  matrix.shutdown(PLAYER_1, 1);
  matrix.shutdown(PLAYER_2, 1);
  
  matrix.clearDisplay(PLAYER_1);
  matrix.clearDisplay(PLAYER_2);

  gameStarted = 0;//reset game

  long highscore;
  readEEPROM(highscore);

  if (score > highscore)
    activeDisplayLCD = NEW_HIGHSCORE;
  else
    activeDisplayLCD = FINAL_SCREEN;

  while (millis() - msgStartTime < 5000);
  
  
  
  
  

}
//display that the score achieved is a highscore and store it in eeprom
void displayNewHighscore()
{
  long msgStartTime = millis();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IT'S A HIGHSCORE");
  lcd.setCursor(6, 1);
  lcd.print(score);
  writeEEPROM(score);

  activeDisplayLCD = FINAL_SCREEN;
  while (millis() - msgStartTime < 5000);
}

//display final screeen with option to play again or exit to menu
void displayFinalScreen()
{
  long msgStartTime = millis();
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("GAME OVER!");
  lcd.setCursor(1, 1);
  lcd.print("PLAY AGAIN ");
  lcd.print("MENU");

  playerActive = PLAYER_1;
  pressed[playerActive] = 0;
  
  bool valSW = true;
  bool cursorPosition = false;
  poz positions[2] = {{0, 1}, {11, 1}};
  
  while (millis() - msgStartTime < 2000);//wait for message to be displayed not to get accidental input to joystick

  while (valSW)
  {
    valSW = digitalRead(pinSW[playerActive]);
    if (!valSW)
      break;

    byte direction = scanJoyStickMove();
    if (direction == LEFT || direction == RIGHT)
      cursorPosition = !cursorPosition;

    lcd.setCursor(positions[cursorPosition].col, positions[cursorPosition].lin);
    lcd.write(byte(0));
    lcd.setCursor(positions[!cursorPosition].col, positions[!cursorPosition].lin);
    lcd.print(' ');
    
    
  }
  pressed[playerActive] = 1;
  //Serial.print(cursorPosition);
  if (cursorPosition == true)
    activeDisplayLCD = MENU;
  else
    activeDisplayLCD = PLAY;
  lcd.clear();
//  Serial.print(activeDisplayLCD);
}
