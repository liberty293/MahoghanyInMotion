/*---------------Libraries-----------------------------*/
#include <Arduino.h>
#include <A4988.h>
#include <SPI.h>
#include <AccelStepper.h>
/*---------------Module Defines-----------------------------*/
#define REDLED 23
#define BLUELED 22
#define WAITINGLED 21
#define IN3_R 19 //controls right wheel dir blue
#define IN1_L 18 //controls left wheel dir green
#define IN4_R 17 //controls right wheel dir yellow 
#define IN2_L 16 //controls left wheel dir orange
#define ENA_L 3 //left wheel PWM black
#define ENB_R 4 //right wheel PWM white
#define STEP 14 // for stepper motor orange
#define DIR 15 // for stepper motor yellow
#define TEAMPOTPIN A6
#define speed_value 60
#define GGTIME 250000000
//#define GGTIME 20000000

#define Line_Threshold 4.0 //FILL THIS AND BELOW IN! 3.5-4.4
#define Left_Line 10
#define LeftCenter_Line 9
#define Center_Line 8
#define RightCenter_Line 7
#define Right_Line 6

#define MS1 10
#define MS2 11
#define MS3 12
uint32_t MoveToSheepTime = 1000; //Time in miliseconds
uint32_t TurnTime90 = 500;
uint32_t TurnTIme45 = 250;


/*---------------Module Function Prototypes-----------------*/
//handle state functions
void handleWaiting(void);
void handleGoToSheep(void);
void handleGoToRedSheep(void);
void handleAtSheep(void);
void handleGoToLine(void);
void handleGG(void);

//move robot functions
void driveForward(int speed);
void driveToSheep(int speed);
void driveBackward(void);
void stopDriving(void);
void stopAllMotors(void);
void turnRight(int speed);
void turnLeft(int speed);
void lineFollow(void);
void openDoor(void);
void closeDoor(void);

//additional useful functions
int potRead(int pin);
bool OnLine(int line);




/*---------------State Definitions--------------------------*/
typedef enum {WAITING, GO_TO_SHEEP_RED, GO_TO_SHEEP,GG, AT_SHEEP, GO_TO_LINE, LINE_FOLLOW
, AT_INTERSECT} States_t;


/*---------------Module Variables---------------------------*/
IntervalTimer GGtimer;
States_t state;
int potval;
int teamcolorval;
AccelStepper doorStepper(1, STEP, DIR);
uint32_t currentmillis;
bool isRed = false;
int intersectNum = 0;

/*---------------Main Functions----------------*/

void setup() {
  pinMode(ENA_L, OUTPUT);
  pinMode(ENB_R, OUTPUT);
  pinMode(IN1_L, OUTPUT);
  pinMode(IN2_L, OUTPUT);
  pinMode(IN3_R, OUTPUT);
  pinMode(IN4_R, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(REDLED, OUTPUT);
  pinMode(WAITINGLED, OUTPUT);
  pinMode(TEAMPOTPIN, INPUT);
  pinMode(Left_Line,INPUT);
  pinMode(LeftCenter_Line, INPUT); 
  pinMode(Center_Line,INPUT) ;
  pinMode(RightCenter_Line,INPUT) ;
  pinMode(Right_Line,INPUT) ;
  state = WAITING;
  GGtimer.begin(handleGG,GGTIME);
  doorStepper.setMaxSpeed(1000);
  doorStepper.setSpeed(1000);
  Serial.begin(9600);
 // stepper.begin(1, 1);

}
void loop(){
//  closeDoor();
 // Serial.println(analogRead(Left_Line));
 // lineFollow();
//turnLeft(speed_value);
 //doorStepper.runSpeed();
 //Serial.println(doorStepper.currentPosition());
  switch (state) {
    case WAITING:
      handleWaiting();
      break;
    // case GO_TO_SHEEP:
    //   handleGoToSheep();
    //   break;
    // case GO_TO_SHEEP_RED:
    //   handleGoToRedSheep();
    // case AT_SHEEP:
    //   handleAtSheep();
    //   break;
    case GO_TO_LINE:
      handleGoToLine();
    case LINE_FOLLOW:
      lineFollow();
    case AT_INTERSECT:
      if (intersectNum ==1){ //ignore the first intersection
        //if(isRed) turnLeft(speed_value);
        //else turnRight(speed_value);
        // currentmillis = millis();
        // if(millis() - currentmillis > TurnTime90){
        //   state=LINE_FOLLOW;
        // }
        driveForward(speed_value);
      }

        else if (intersectNum ==2){
        if(isRed) turnRight(speed_value);
        else turnLeft(speed_value);
        currentmillis = millis();
        if(millis() - currentmillis > TurnTime90){
          state=LINE_FOLLOW;
        }
      }
        
        else if (intersectNum ==3){
        if(isRed) turnLeft(speed_value);
        else turnRight(speed_value);
        currentmillis = millis();
        if(millis() - currentmillis > TurnTIme45){
          state=LINE_FOLLOW;
        }
      }

        else if(intersectNum>=4){
          driveForward(speed_value);
        }         
      
    case GG:
      stopDriving();
      break;
    default:    // Should never get into an unhandled state
      Serial.println("What is this I do not even...");
  }
}



/*----------------Module Functions--------------------------*/
int potRead(int pin) { // reads a potentiometer value and returns a value from 0 to 100
  potval = analogRead(pin);            
  return map(potval,0,1023,0,100);
}

void handleWaiting(void){
    digitalWrite(REDLED,LOW);
    digitalWrite(BLUELED,LOW);
    digitalWrite(WAITINGLED,HIGH);
    if (potRead(TEAMPOTPIN)>=75){
       isRed = false;
       digitalWrite(BLUELED,HIGH);
       digitalWrite(WAITINGLED,LOW);
     }
    if (potRead(TEAMPOTPIN)<=25){
       isRed = true;
       digitalWrite(REDLED,HIGH);
       digitalWrite(WAITINGLED,LOW);
     }
     state = GO_TO_LINE;

}

void handleGoToLine(void){
  if(OnLine(Center_Line))
    state = LINE_FOLLOW;
}


void handleGG(void){ // all motors off, all LEDs off
    digitalWrite(REDLED,LOW);
    digitalWrite(BLUELED,LOW);
    digitalWrite(WAITINGLED,LOW);
    state = GG;
}

void driveForward(int speed){//moves robot forward
  Serial.println("fwd");
  digitalWrite(IN1_L, LOW);
  digitalWrite(IN2_L, HIGH);
  digitalWrite(IN3_R, HIGH);
  digitalWrite(IN4_R, LOW);
  analogWrite(ENA_L, 600);
  analogWrite(ENB_R, 600);
}


void driveBackward(int speed){ // moves robot backward
  Serial.println("backwards");
  digitalWrite(IN2_L, HIGH);
  digitalWrite(IN1_L, LOW);
  digitalWrite(IN4_R, HIGH);
  digitalWrite(IN3_R, LOW);
  analogWrite(ENA_L, speed);
  analogWrite(ENB_R, speed);
}

void stopDriving(void){ // stops driving motors
  Serial.println("Good Game");
  analogWrite(ENA_L, 0);
  analogWrite(ENB_R, 0);
  
}

void turnLeft(int speed){ // turns robot right
  digitalWrite(IN3_R, HIGH);
  digitalWrite(IN4_R, LOW);
  digitalWrite(IN1_L, HIGH);
  digitalWrite(IN2_L, LOW);
  analogWrite(ENA_L, speed);
  analogWrite(ENB_R, speed);
}

void turnRight(int speed){ // turns robot left Right wheel forward; left wheel back
  digitalWrite(IN4_R, HIGH);
  digitalWrite(IN3_R, LOW);
  digitalWrite(IN2_L, HIGH);
  digitalWrite(IN1_L, LOW);
  analogWrite(ENA_L, speed);
  analogWrite(ENB_R, speed);
}

void lineFollow(void){ // line following
//if center sensor reads black move forward
//if corner sensors read black then we're at an intersection
if(OnLine(Center_Line))
{
  driveForward(speed_value);
  if(OnLine(Right_Line) || OnLine(Left_Line))
    {
      intersectNum++;
      state = AT_INTERSECT;
      //if(isRed) turnLeft(speed_value);
      //else turnRight(speed_value);
    }

}

else //if not on a line, turn towards line
{
//if left sensor reads black, turn left
  if(OnLine(LeftCenter_Line))
    turnLeft(speed_value);

//if right sensor reads black, turn right
  if(OnLine(RightCenter_Line))
    turnRight(speed_value);
}
}

bool OnLine(int line){
  if (digitalRead(line) == 1)
    return true;
  else
    return false;
}