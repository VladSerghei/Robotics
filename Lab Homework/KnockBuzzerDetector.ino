const int buttonPin=2;
const int buzzPin=A0;
const int tonePin=9;
const int threshold=500;
int buttonState=0;
int buzzVal=0;
int pushTime=0;
int crTime=0;
int buzzSet=0;
void setup() {
  // put your setup code here, to run once:
pinMode(buttonPin,INPUT_PULLUP);
pinMode(buzzPin,INPUT);
pinMode(tonePin,OUTPUT);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
buttonState=digitalRead(buttonPin);
buzzSet=!buttonState;
buzzVal=analogRead(buzzPin);


if(buzzVal>threshold&&pushTime==0)
{
  pushTime=millis();
  crTime=pushTime;
}
if(crTime!=0)
{
  crTime=millis();
}
if(crTime-pushTime>5000)
{
  tone(tonePin,2000);
}
if(buzzSet==HIGH)
{
  noTone(tonePin);
  crTime=0;
  pushTime=0;
}
/*Serial.print(buzzVal);
Serial.print(' ');
Serial.print(pushTime);
Serial.print(' ');
Serial.print(buzzSet);
Serial.print(' ');
Serial.println(crTime);*/
}
