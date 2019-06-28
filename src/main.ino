#include <Arduino.h>
#include <Encoder.h>
#include <L298N.h>
#include <Wire.h>   // wire library for i2c communication

int encoderMultiplier = 65;   // equals one revolution  ... not accurate at the moment but aprox
int revolutionsToGo = 10;     // how many revolutions should the motor go before reversing
int direction = 0;            // initial motor direction

/* i2c settings - the i2c pins on the nano are A5 for SCL and A4 for SDA - has to be unique for each device on the bus */

#define ANSWERSIZE 7
byte answer[3] = {0,100,0};
//answer = "0-100-0"; // direction - speed - currentposition

byte slaveAdd = 0x01; //i2c address of device

/* encoder library https://github.com/PaulStoffregen/Encoder */
// settings for encoder
#define enc1 3
#define enc2 2
long oldPosition  = 0;
Encoder myEnc(enc1, enc2);

/* enA pin gets pwm from 0 to 255 for speed - here the initial speed is set
the library abstracts this function
https://github.com/AndreaLombardo/L298N
*/
// settings for hbridge
#define enA 9
#define in1 6
#define in2 7
#define button 4
int speed = 100;          // initial motor speed
L298N motor(enA, in1, in2);

void setup() {
  Wire.begin(slaveAdd);
  Wire.onRequest(sendEvent);    // data sending to MEGA
  Wire.onReceive(receiveEvent); // data received from MEGA

  Serial.begin(115200);
  Serial.print("MovingArt - Nano i2c Address: ");
  Serial.println();

  motor.setSpeed(speed);
  myEnc.write(0);             // reset encoder to zero
}

void loop() {
  // moving the motor -> motor.forward() or motor.backward();;
  // setting motor speed -> motor.setSpeed(theSpeed);

  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
  if(direction == 0){
    // inspect motor movement
    if(newPosition >= encoderMultiplier*revolutionsToGo){
      direction = 1;
      motor.backward();
      // change rotation directon
    };
  }else if(direction == 1){
    if(newPosition <= encoderMultiplier*revolutionsToGo*-1){
      direction = 0;
      motor.stop();
      // send status
    };
  }
}

void receiveEvent(int countToRead) {
  // this is called when the Master Mega sends a request to the nano
  while (0 < Wire.available()) {
    byte command = Wire.read();       // read command from mega

    if(command == 0){
      Serial.println("Motor stopping");
      motor.stop();
    }else if(command == 1){
        if(motor.isMoving()){
          Serial.println("Motor already moving - do nothing");
          // but should answer with current position and status
        }else{
          Serial.println("Motor starting");
            if(direction == 0){
              // moving forward
              motor.forward();
            }else if(direction == 1){
              // moving backward
              motor.backward();
            }else{
              // safety if anything went wrong
              direction = 0;
              motor.stop();
            };
        }
    }else{};

  }
  Serial.println("Received event From Mega");

}

void sendEvent() {
  answer[0] = direction;     // set direction to current direction
  answer[1] = speed;         // set speed to current speed
  answer[2] = oldPosition;

  for (int i=0; i<3; i++)
  {
    Wire.write(answer[i]);  //data bytes are queued in local buffer
  }
  Wire.endTransmission();
  Serial.println("Respond back to Mega");
}
