#include <EEPROM.h>

#include <LiquidCrystal.h>
#define menu 4
#define game 0
#define endGame 3
#define options 1
#define highScoreMenu 2

#define left 1
#define right 2
#define up 3
#define down 4

#define changeName 0
#define back 1
#define changeLevel 2

#define minLvl 0
#define maxLvl 9

//initial variables
const int D4 = 10;
const int D5 = 11;//LCD related
const int D6 = 12;
const int D7 = 13;
const int E = 9;
const int RS = 8;
LiquidCrystal lcd(RS,E,D4,D5,D6,D7);

const int pinX = A0;//joystick related
const int pinY = A1;
const int pinSW = 7;
const int highThreshold = 650;
const int lowThreshold = 450;
int moved = 0;
int pressed = 0;

int activeDisplay = menu;

int index = 0;
String Name = String("none");

struct Poz
{
  int col;
  int lin;
  
};
typedef struct Poz poz;

int lives = 3;
int startLevel = 0;
int level = 0;
int score = 0;
long lastMillis = -1;
int highScore = 0;
vlad
//functions and preparation
byte arrow[8] = {//custom character for  cursor
  B10000,
  B01000,
  B00100,
  B00010,
  B00100,
  B01000,
  B10000,
};
int scanJoyStick(int &moved)
{
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);
  if (xValue > highThreshold && moved == 0)
    {
      moved = 1;
      return right;
    }
   if (xValue < lowThreshold && moved == 0)
    {
      moved = 1;
      return left;
    }
    if (yValue > highThreshold && moved == 0)
    {
      moved = 1;
      return down;    
    }
   if (yValue < lowThreshold && moved == 0)
    {
      moved = 1;
      return up;   
    }
   if (xValue > lowThreshold && xValue < highThreshold && yValue > lowThreshold && yValue < highThreshold)
      moved=0;
return 0;
}

poz cursorPositionsMain[3]={{0, 0}, {7, 0}, {0, 1}};//cursor positions in the main menu
poz cursorPositionsOptions[3]={{0, 0}, {11, 0}, {0, 1}};
int cursorActualMain=0;
int cursorActualOptions=0;

//program
void setup() 
{
lcd.createChar(0, arrow);
lcd.begin(16, 2);
pinMode(pinX, INPUT);
pinMode(pinY, INPUT);
pinMode(pinSW, INPUT_PULLUP);
Serial.begin(9600);
}

void loop() 
{
  
  
  if (activeDisplay == menu)
    displayMenu();
  else if (activeDisplay == game)
    displayGame(startLevel);
  else if (activeDisplay == endGame)
    displayEndGame();
  else if (activeDisplay == options)
    displayOptions();
  else if (activeDisplay == highScoreMenu)
    displayHighScore();  
  

  
  
  


}
void displayMenu()//displaying the main menu
{
  
  lcd.setCursor(1, 0);
  lcd.print("Play");
  lcd.setCursor(8, 0);
  lcd.print("Settings");
  lcd.setCursor(1, 1);
  lcd.print("Highscore");
  lcd.setCursor(cursorPositionsMain[cursorActualMain].col, cursorPositionsMain[cursorActualMain].lin);
  lcd.write(byte(0));
  
  
  int cursorMove = scanJoyStick(moved);//change menu selection
  if (cursorMove == left)
  {
    if (cursorActualMain == 0)
      cursorActualMain = 2;
    else
      cursorActualMain = cursorActualMain - 1;
    lcd.clear();
  }
  else if (cursorMove == right)
  {
    if (cursorActualMain == 2)
      cursorActualMain = 0;
    else
      cursorActualMain = cursorActualMain + 1;
    lcd.clear();
  }

  int valSW = digitalRead(pinSW);
  if(valSW == 0 && pressed == 0)
  {
    lcd.clear();
    activeDisplay = cursorActualMain;
    pressed = 1;
    return;
  }
  if(valSW == 1 && pressed == 1)
  {
    pressed = 0;
  }

}
void displayGame(int startingLevel)//displaying the game
{
  if (lastMillis == -1)//when game starts
    {
      lcd.clear();
      score = 0;
      level=startingLevel;
      lastMillis = millis();
    }
  if (level - startingLevel > 1)//when game ends
  {
      lastMillis = -1;
      readEEPROM(highScore);
      if (score > highScore)
        {
          clearEEPROM();
          writeEEPROM(score);
        }
      lcd.clear();
      activeDisplay = endGame;
      delay(100);
      return;
  }
  
  long currentTime = millis();
  if (currentTime - lastMillis > 5000)
  {
    level += 1;
    lastMillis = millis();
  }
  score = level * 3;

  
  lcd.setCursor(0, 0);
  lcd.print("Lives:");
  lcd.print(lives);
  lcd.print(" ");
  lcd.print("Level:");
  lcd.print(level);

  lcd.setCursor(8, 1);
  lcd.print("Score:");
  lcd.print(score);
}
void displayEndGame()// displaying the endgame screen
{
  
  if (lastMillis == -1)//when entering function
    {
      index=0;
      lcd.clear();
      lastMillis = millis();
      lcd.setCursor(0, 1);
      lcd.print("Press button to exit");
    }
    
  int SwValue = digitalRead(pinSW);//to exit
  if (SwValue == 0 && pressed ==0)
    {
      index=0;
      lastMillis=-1;
      activeDisplay = menu;
      pressed = 1;
      lcd.clear();
      return;
    }
  if (SwValue == 1 && pressed == 1)
    {
      pressed=0;
    } 
  long currentTime = millis();
  if (currentTime - lastMillis > 1000)
  {
    
    if (index == strlen("Press button to exit"))
      {
        index = 0;
        lcd.home();
      }
    else
      {
        index = index + 1;
        lcd.scrollDisplayLeft();
      }
    lastMillis=millis();
    
  }
  

  lcd.setCursor(index, 0);
  lcd.print("Congratulations!");
  
}

void displayHighScore()//displaying the highscore menu
{
  readEEPROM(highScore);
  int SWvalue = digitalRead(pinSW);
  lcd.setCursor(0, 0);
  lcd.print("Highscore:");
  lcd.print(highScore);

  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.print("Exit");

  if (SWvalue == 0 && pressed == 0)
    {
      activeDisplay = menu;
      pressed = 1;
      lcd.clear();
      return;
    }
  if (SWvalue == 1 && pressed == 1)
  {
    pressed = 0;
  }
}

void displayOptions()//displaying the options menu
{
  lcd.setCursor(1, 0);
  lcd.print("Name:");
  lcd.print(Name);

  lcd.setCursor(12, 0);
  lcd.print("Exit");
  
  lcd.setCursor(1, 1);
  lcd.print("Start level:");
  lcd.print(startLevel);

  lcd.setCursor(cursorPositionsOptions[cursorActualOptions].col, cursorPositionsOptions[cursorActualOptions].lin);
  lcd.write(byte(0));
  
  
  int cursorMove = scanJoyStick(moved);
  if (cursorMove == left)
  {
    if (cursorActualOptions == 0)
      cursorActualOptions = 2;
    else
      cursorActualOptions = cursorActualOptions - 1;
    lcd.clear();
  }
  else if (cursorMove == right)
  {
    if (cursorActualOptions == 2)
      cursorActualOptions = 0;
    else
      cursorActualOptions = cursorActualOptions + 1;
    lcd.clear();
  }

  int SWvalue = digitalRead(pinSW);
  if (SWvalue == 0 && pressed == 0)
  {
    pressed = 1;
    if (cursorActualOptions == changeName)
    {
      getName(Name);
      
    }
    else if (cursorActualOptions == back)
    {
      activeDisplay = menu;
      cursorActualOptions = 0;
      lcd.clear();
      delay(100);
      return;
    }   
  }
  if (SWvalue == 1 && pressed == 1)
  {
    pressed = 0;
  }
  if (cursorActualOptions == changeLevel)
  {
      if (cursorMove == up)
    {
      if (startLevel == maxLvl)
        startLevel = minLvl;
      else
        startLevel = startLevel + 1;
      lcd.clear();
    }
    else if (cursorMove == down)
    {
      if  (startLevel == minLvl)
        startLevel = maxLvl;
      else
        startLevel = startLevel - 1;
      lcd.clear();
    }
  }
  
  
}
void getName(String &str)
{
  
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Enter name");
  lcd.setCursor(0, 1);
  lcd.print("Max 5 characters");
  

  while (Serial.available() <= 0)
    delay(1);
  str=Serial.readString();
  str=str.substring(0,4);
  
  lcd.clear();
  return;
  
}

void writeEEPROM(int &x)
{
  byte *v = (byte*)(void*)&x;
  int addr = 0;
  for (int i = 0; i < sizeof(x); i++)
          EEPROM.write(addr++, *v++);
 
}
void readEEPROM(int &x)
{
  byte *v = (byte*)(void*)&x;
  int addr = 0;
  for (int i = 0; i < sizeof(x); i++)
          *v++ = EEPROM.read(addr++);
 
}
void clearEEPROM()
{
  for (int i = 0 ; i < EEPROM.length() ; i++) 
  {
    EEPROM.write(i, 0);
  }
}
