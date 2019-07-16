#include <Arduino.h>
#include <Encoder.h>
#include <L298N.h>
#include <Wire.h>   // wire library for i2c communication

long encoderMultiplier = 7000;  // equals one revolution  ... not accurate at the moment but aprox
int revolutionsToGo;            // how many revolutions should the motor go before reversing
int direction = 1;            // initial motor direction
bool started = false;
/* i2c settings - the i2c pins on the nano are A5 for SCL and A4 for SDA - has to be unique for each device on the bus */

#define ANSWERSIZE 7

byte receivedcommand[4];
//answer = "0-100-0"; // direction - speed - currentposition

//byte slaveAdd = 0x01; //i2c address of device
//byte slaveAdd = 0x02; //i2c address of device
//byte slaveAdd = 0x03; //i2c address of device
//byte slaveAdd = 0x04; //i2c address of device
//byte slaveAdd = 0x05; //i2c address of device
//byte slaveAdd = 0x06; //i2c address of device
//byte slaveAdd = 0x07; //i2c address of device...
//byte slaveAdd = 0x08; //i2c address of device
//byte slaveAdd = 0x09; //i2c address of device
//byte slaveAdd = 0x0A; //i2c address of device
//byte slaveAdd = 0x0B; //i2c address of device
//byte slaveAdd = 0x0C; //i2c address of device

/* encoder library https://github.com/PaulStoffregen/Encoder */
// settings for encoder
#define enc1 2
#define enc2 3
long oldPosition  = 0;
Encoder myEnc(enc1, enc2);

/* enA pin gets pwm from 0 to 255 for speed - here the initial speed is set
the library abstracts this function
https://github.com/AndreaLombardo/L298N
*/
// settings for hbridge
#define enA 9
#define in1 6
#define in2 5
#define button 4
int speed = 255;          // initial motor speed
L298N motor(enA, in1, in2);

void setup() {
  Wire.begin(slaveAdd);
  Wire.onRequest(sendEvent);    // data sending to MEGA
  Wire.onReceive(receiveEvent); // data received from MEGA

  Serial.begin(115200);
  Serial.print("MovingArt - Nano i2c Address: ");
  Serial.println(slaveAdd);

  motor.setSpeed(speed);
  myEnc.write(0);             // reset encoder to zero
  motor.stop();
  started = false;

  Serial.print("moving: "); Serial.println(motor.isMoving());
}

void loop() {
  // moving the motor -> motor.forward() or motor.backward();;
  // setting motor speed -> motor.setSpeed(theSpeed);

    long newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      //Serial.println(newPosition);
    }

  if(started){
    //Serial.println("started");
    if(direction == 1){
      // inspect motor movement
      if(newPosition >= encoderMultiplier*revolutionsToGo){
        direction = 0;
        motor.backward();
        // change rotation directon
      };
    }else if(direction == 0){
      if(newPosition <= 0){
        direction = 1;
        motor.stop();
        // send status

      };
    }
  }


}

void receiveEvent() {
  // this is called when the Master Mega sends a request to the nano
  int x = 0;
  while (0 < Wire.available()) {
    byte command = Wire.read();       // read command from mega
    receivedcommand[x]=command;
    x++;
  }
  Serial.println("Received event From Mega:");
  for (int i=0; i<4; i++)
  {
    Serial.print(receivedcommand[i]);
    Serial.print("|");
  }
  Serial.println();

  if(receivedcommand[0] == 0){
    Serial.println("Motor stopping");
    motor.stop();
    started = false;
  }else if(receivedcommand[0] == 1){
    Serial.println("Motor turning forward");
    Serial.print("moving: "); Serial.println(motor.isMoving());
      if(motor.isMoving() == 1){
        Serial.println("Motor already moving - do nothing");

        started = true;
      }else{
        Serial.println("Motor starting");
        revolutionsToGo = receivedcommand[1];
        speed = receivedcommand[2];
        motor.setSpeed(speed);
        direction = 1;
        motor.forward();
        started = true;

      }
  }else if(receivedcommand[0] == 2){
    Serial.println("Motor Status request");

  }else{};

}

void sendEvent() {
  Serial.println("Respond back to Mega");
  byte answer[3];
  answer[0] = direction;     // set direction to current direction
  answer[1] = slaveAdd;      // motor address
  answer[2] = motor.isMoving();       // motor status is stopped

  for (int i=0; i<3; i++)
  {
    Wire.write(answer[i]);
    Serial.println(answer[i]);
  }
}
