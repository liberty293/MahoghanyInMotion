/*---------------Libraries-----------------------------*/
#include <Arduino.h>
#include <A4988.h>
#include <SPI.h>
#include <Metro.h>

/*---------------Module Defines-----------------------------*/
#define REDLED 23
#define BLUELED 22
#define WAITINGLED 21
#define IN3_R 19 //controls right wheel dir blue
#define IN1_L 18 //controls left wheel dir green
#define IN4_R 17 //controls right wheel dir yellow 
#define IN2_L 16 //controls left wheel dir orange
#define ENA_L 3 //left wheel PWM black
#define ENB_R 4 //right wheel PWM purple
#define TEAMPOTPIN A6
#define speed_value 60
#define GGTIME 250000000
#define Line_Threshold 4.0 //FILL THIS AND BELOW IN! 3.5-4.4
#define Left_Line 10
#define LeftCenter_Line 9
#define Center_Line 8
#define RightCenter_Line 7
#define Right_Line 6
#define TURN_TIME_INTERVAL 500


/*---------------Module Function Prototypes-----------------*/
//handle state functions
void checkGlobalEvents(void);
void handleWaiting(void);
void handleGG(void);

//move robot functions
void driveForward(int speed);
void driveBackward(void);
void stopDriving(void);
void stopAllMotors(void);
void turnRight(int speed);
void turnLeft(int speed);
//void lineFollow(void);

//additional useful functions
int potRead(int pin);
uint8_t OnLine(int line);
unsigned char TestForLine(void);
void RespToLine(void);
unsigned char TestTurnTimerExpired(void);
void RespTurnTimerExpired(void);


/*---------------State Definitions--------------------------*/
typedef enum {WAITING, IN_HOME, PLOWING, CW, CCW, LINE_FOLLOW, GG
} States_t;


/*---------------Module Variables---------------------------*/
IntervalTimer GGtimer;
States_t state;
static Metro metTimer = Metro(TURN_TIME_INTERVAL);
int potval;
int teamcolorval;
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
  Serial.begin(9600);


}
void loop(){
  switch (state) {
    case WAITING:
      handleWaiting();
      break;
    case IN_HOME:
      driveForward(speed_value);
      break;
    case PLOWING:
      digitalWrite(REDLED,HIGH);
      digitalWrite(BLUELED,HIGH);
      digitalWrite(WAITINGLED,HIGH);
      driveForward(speed_value);
      break;
    case LINE_FOLLOW:
      driveForward(speed_value);
      break;
    case CW:
      turnRight(speed_value);
      break;
    case CCW:
      turnLeft(speed_value); 
      break;     
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
     state = PLOWING;

}

// void handleGG(void){ // all motors off, all LEDs off
//     digitalWrite(REDLED,LOW);
//     digitalWrite(BLUELED,LOW);
//     digitalWrite(WAITINGLED,LOW);
//     state = GG;
// }

void driveForward(int speed){//moves robot forward
  Serial.println("fwd");
  digitalWrite(IN1_L, LOW);
  digitalWrite(IN2_L, HIGH);
  digitalWrite(IN3_R, HIGH);
  digitalWrite(IN4_R, LOW);
  analogWrite(ENA_L, speed);
  analogWrite(ENB_R, speed);
}


// void driveBackward(int speed){ // moves robot backward
//   Serial.println("backwards");
//   digitalWrite(IN2_L, HIGH);
//   digitalWrite(IN1_L, LOW);
//   digitalWrite(IN4_R, HIGH);
//   digitalWrite(IN3_R, LOW);
//   analogWrite(ENA_L, speed);
//   analogWrite(ENB_R, speed);
// }

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

// void lineFollow(void){ // line following
//   driveForward(speed_value);
// }

uint8_t OnLine(int line){
  if (digitalRead(line) == 1)
    return 1;
  else
    return 0;
}

uint8_t TestForLine(void) {
  // To be written in Part 2
  return OnLine(Center_Line);
}

void RespToLine(void) {
    if (state == IN_HOME) { //havent left home yet, keep driving
      state = PLOWING;
  }
    if (state == PLOWING) { //we've hit the first line, must turn
      if (isRed == true) {
      //turn right until the left sensor has hit line 
        // while (!OnLine(LeftCenter_Line)) {
        //   turnRight(speed_value);
        // }
        state = CCW;

    } else {
      //turn left until the right sensor has hit line 
        // while (!OnLine(RightCenter_Line)) {
        //   turnLeft(speed_value);
        // }
        state = CW;
    }
  }
    if (state == LINE_FOLLOW) { //we are line following
      if (isRed == true) {
      state = CW;
    } else {
      state = CCW;
    }
  }
}

uint8_t TestTurnTimerExpired(void) {
  return (uint8_t) metTimer.check();
}

void RespTurnTimerExpired(void) {
  metTimer.reset();
  state = LINE_FOLLOW;
}

void checkGlobalEvents(void) {
  if (TestForLine()) RespToLine();
  if (TestTurnTimerExpired()) RespTurnTimerExpired();

}

