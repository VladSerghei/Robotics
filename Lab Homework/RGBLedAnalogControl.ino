const int redOut=11;
const int greenOut=10;
const int blueOut=9;
const int redIn=A0;
const int greenIn=A1;
const int blueIn=A2;
int red=0;
int green=0;
int blue=0;


void setup() {
  // put your setup code here, to run once:
pinMode(redOut,OUTPUT);
pinMode(greenOut,OUTPUT);
pinMode(blueOut,OUTPUT);
pinMode(redIn,INPUT);
pinMode(greenIn,INPUT);
pinMode(blueIn,INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  red=analogRead(redIn);
  green=analogRead(greenIn);
  blue=analogRead(blueIn);
  red=map(red,0,1023,0,255);
  green=map(green,0,1023,0,255);
  blue=map(blue,0,1023,0,255);
analogWrite(redOut,red);
analogWrite(greenOut,green);
analogWrite(blueOut,blue);
}
