#include <Servo.h>
#include <LiquidCrystal_I2C.h>
enum CarState {
  NORMAL,
  MOTOR_OFF,
  OBSTACLE,
  SENSOR_ERROR
};
CarState carState = NORMAL;
LiquidCrystal_I2C lcd(0x27, 16, 2);
int ser = 8;
Servo srs;
int potspeedpin = A0;
int potsteering = A1;
int potspeed;
int potvalue;
int steeringValue;
int trigr = 7;
int echor = 4;
int trigl = 10;
int echol = 11;
int IN[] = {9, 6, 12, 13};
int ENA = 3;
int ENB = 5;
int buttonDir = 2;
int buttonprinting = A2;
int buttonMotor = A3;
int lastButtonState2 = HIGH;
volatile bool dir = true;
bool motorEnabled = true;
float distancer;
float distancel;
int angle;
int currentPWM = 0;
int rpm;
int pwm;
float wheelRadius = 0.0325;
float vehicleSpeed;

void changeDirection();
void forward();
void back();
void stopCar();
float calcdistance(int trig, int echo);
void displayData();

void setup(){

  srs.attach(ser);
  pinMode(potspeedpin, INPUT);
  pinMode(potsteering, INPUT);
  pinMode(trigr, OUTPUT);
  pinMode(echor, INPUT);
  pinMode(trigl, OUTPUT);
  pinMode(echol, INPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(IN[i], OUTPUT);
  }
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(buttonDir, INPUT_PULLUP);
  pinMode(buttonMotor, INPUT_PULLUP);
  pinMode(buttonprinting, INPUT_PULLUP);
  Serial.begin(9600);
  attachInterrupt(
  digitalPinToInterrupt(buttonDir),
    changeDirection,
    FALLING
  );
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Steering Car");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(1500);
  lcd.clear();
}

void loop() {

  int buttonState2 = digitalRead(buttonMotor);
  if (lastButtonState2 == HIGH && buttonState2 == LOW) {
    motorEnabled = !motorEnabled;
    delay(50);
  }

  lastButtonState2 = buttonState2;

  distancer = calcdistance(trigr, echor);
  distancel = calcdistance(trigl, echol);

  potspeed = analogRead(potspeedpin);
  pwm = map(potspeed, 0, 1023, 0, 255);
  rpm = map(potspeed, 0, 1023, 0, 600);

  steeringValue = analogRead(potsteering);
  angle = map(steeringValue, 0, 1023, 0, 180);
  srs.write(angle);

  vehicleSpeed = 2 * 3.1415 * wheelRadius * (rpm / 60.0);

  if (currentPWM < pwm) {
    currentPWM += 5;
    if (currentPWM > pwm) {
      currentPWM = pwm;
    }
  }
  else if (currentPWM > pwm) {
    currentPWM -= 5;
    if (currentPWM < pwm) {
      currentPWM = pwm;
    }
  }
  if (distancer == -1 || distancel == -1) {
    carState = SENSOR_ERROR;
    stopCar();
  }
  else if (distancer <= 30 || distancel <= 30) {
    carState = OBSTACLE;
    stopCar();
  }
  else if (!motorEnabled) {
    carState = MOTOR_OFF;
    stopCar();
  }
  else {
    carState = NORMAL;
    if (dir) {
      forward();
    }
    else {
      back();
    }
  }
  displayData();
int buttonStatePrint = digitalRead(buttonprinting);

if (buttonStatePrint == LOW) {
  displayData();
}
  delay(100);
}
void changeDirection() {
  unsigned long currentTime = millis();
  static unsigned long lastInterruptTime = 0;
  if (currentTime - lastInterruptTime > 250) {
      dir = !dir;
  }
  lastInterruptTime = currentTime;
}

void forward() {
  digitalWrite(IN[0], HIGH);
  digitalWrite(IN[1], LOW);
  digitalWrite(IN[2], HIGH);
  digitalWrite(IN[3], LOW);
  analogWrite(ENA, currentPWM);
  analogWrite(ENB, currentPWM);
}

void back() {

  digitalWrite(IN[0], LOW);
  digitalWrite(IN[1], HIGH);
  digitalWrite(IN[2], LOW);
  digitalWrite(IN[3], HIGH);
  analogWrite(ENA, currentPWM);
  analogWrite(ENB, currentPWM);
}

void stopCar() {

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN[0], LOW);
  digitalWrite(IN[1], LOW);
  digitalWrite(IN[2], LOW);
  digitalWrite(IN[3], LOW);
  currentPWM = 0;
}

float calcdistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long time = pulseIn(echo, HIGH, 30000);
  if (time == 0) {
    return -1;
  }
  float distance = (time * 0.0343) / 2;
  return distance;
}
void displayData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RPM:");
  lcd.print(rpm);
  delay(1500);
  lcd.print(" SPD:");
  lcd.print(int(vehicleSpeed));
  delay(1500);
  lcd.setCursor(0, 1);
  if (carState == NORMAL) {
    lcd.print("NORMAL");
    delay(1500);
  }
  else if (carState == MOTOR_OFF) {
    lcd.print("MOTOR OFF");
    delay(1500);
  }
  else if (carState == OBSTACLE) {
    lcd.print("OBSTACLE");
    delay(1500);
  }
  else{
    lcd.print("SENSOR ERR");
    delay(1500);
  }
}