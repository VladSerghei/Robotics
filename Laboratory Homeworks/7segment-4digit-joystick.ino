
const int pinSW = 0;
const int pinX = A0;
const int pinY = A1;

int xValue = 0;
int yValue = 0;
int swState = LOW;
int lastSwState = LOW;

int lastBlink=0;
int DPValue=LOW;

int index=0;
int minThreshold= 400;
int maxThreshold= 600;
bool joyMoved=false;

const int pinA = 3;
const int pinB =5 ;
const int pinC = 11;
const int pinD = 9;
const int pinE = 8;
const int pinF = 4;
const int pinG = 12;
const int pinDP = 10;
const int pinD1 = 13;
const int pinD2 =7 ;
const int pinD3 =6 ;
const int pinD4 =2 ;

const int segSize = 8;
const int noOfDigits = 10;
const int noOfDisplays = 4;

int activeDisplay=0;
int digit=0;
int locked=0;

int segments[segSize] = {
pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

byte digitMatrix[noOfDigits][segSize - 1] = {
// a b c d e f g
{1, 1, 1, 1, 1, 1, 0},
{0, 1, 1, 0, 0, 0, 0}, // 1
{1, 1, 0, 1, 1, 0, 1}, // 2
{1, 1, 1, 1, 0, 0, 1}, // 3
{0, 1, 1, 0, 0, 1, 1}, // 4
{1, 0, 1, 1, 0, 1, 1}, // 5
{1, 0, 1, 1, 1, 1, 1}, // 6
{1, 1, 1, 0, 0, 0, 0}, // 7
{1, 1, 1, 1, 1, 1, 1}, // 8
{1, 1, 1, 1, 0, 1, 1} // 9
};

int digits[noOfDisplays] = {
pinD4, pinD3, pinD2, pinD1
};

int num[noOfDisplays]={0,0,0,0};

void displayNumber(byte digit)
{
  for (int i = 0; i < segSize - 1; i++) 
  {
    digitalWrite(segments[i], digitMatrix[digit][i]);
  }

}

void activateDigit(int num)
{
for (int i = 0; i < noOfDisplays; i++) {
digitalWrite(digits[i], HIGH);
}
digitalWrite(digits[num], LOW);
}


void setup() 
{
    // put your setup code here, to run once:
  for (int i = 0; i < segSize; i++) {
  pinMode(segments[i], OUTPUT);
  }
  for (int i = 0; i < noOfDisplays; i++)
  {
  pinMode(digits[i], OUTPUT);
  }

  pinMode(pinSW, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
swState = digitalRead(pinSW);
xValue = analogRead(pinX);
yValue = analogRead(pinY);

if (swState != lastSwState) 
{
  if (swState == LOW) 
  {
    locked=!locked;
  }
}
lastSwState=swState;


if(locked==0)
{
  if (xValue < minThreshold && joyMoved == false) 
  {
    if (activeDisplay > 0) 
    {
      activeDisplay--;
    } 
    else 
    {
      activeDisplay=3;
    }
    joyMoved = true;
    activateDigit(activeDisplay);
  }

  if (xValue > maxThreshold && joyMoved == false) 
  {
    if (activeDisplay < 3) 
    {
      activeDisplay++;
    } 
    else 
    {
      activeDisplay=0;
    }
    joyMoved = true;
    activateDigit(activeDisplay);
   }
   if (xValue >= minThreshold && xValue <= maxThreshold)
   {
    joyMoved = false;
   }
    
}
else
{
  
  if (yValue > maxThreshold && joyMoved == false) 
  {
    if (num[activeDisplay] > 0) 
    {
      num[activeDisplay]--;
    } 
    else 
    {
      num[activeDisplay]=9;
    }
    joyMoved = true;

  }

  if (yValue < minThreshold && joyMoved == false) 
  {
    if (num[activeDisplay] < 9) 
    {
      num[activeDisplay]++;
    } 
    else 
    {
      num[activeDisplay]=0;
    }
    joyMoved = true;
   }
   if (yValue >= minThreshold && yValue <= maxThreshold)
   {
    joyMoved = false;
   }
  
}

for(index=0;index<noOfDisplays;index++)
{
  if(index==activeDisplay)
  {
    activateDigit(index);
    if(locked==1)
    {
      displayNumber(num[index]);
      digitalWrite(segments[segSize - 1], HIGH);
    }
    else
    {
      if(millis()-lastBlink>500)
      {
        DPValue=!DPValue;
        lastBlink=millis();
      }
      
      displayNumber(num[index]);
      digitalWrite(segments[segSize - 1], DPValue);
    }
  delay(5);
  }
  else
  {
  activateDigit(index);
  displayNumber(num[index]);
  digitalWrite(segments[segSize - 1], LOW);
  delay(5);
  }
}
activateDigit(activeDisplay);

Serial.print(num[0]);
Serial.print("  ");
Serial.print(num[1]);
Serial.print("  ");
Serial.print(num[2]);
Serial.print("  ");
Serial.print(num[3]);
Serial.print("  \n");





}
