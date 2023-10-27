// 1. 模拟电子开关门实验：使用ESP32和一个舵机、一个LED灯，设计一个开门关门的模拟装置。
// 实现如下功能 ：
// （1）通电时， 舵机臂回到0度位置；
// （2）用手指触摸指定IO线，舵机开始转向90度位置，同时小灯以每秒5次闪烁；
// （3）保持舵机90度位置，灯常亮，5秒钟后， 舵机臂开始回到0度位置，同时小灯以每秒5次闪烁；
// （4）舵机臂回到0度位置后，小灯常灭。等待下一次手指触摸，重复（2）；  

#include <Arduino.h>
#include <Ticker.h> 

int servoPin = 26, freq = 50, ledChannel = 0, resolution = 10;
int dutyCycle90 = 76.8, dutyCycle0 = 25;
int ledPin = 25;
int touchPin = 27;

void setup() {
  Serial.begin(115200);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(servoPin, ledChannel);
  pinMode(touchPin, INPUT);
  pinMode(ledPin, OUTPUT);

  ledcWrite(ledChannel, dutyCycle0);
}

void loop() {
  if (digitalRead(touchPin) == HIGH) {
    Serial.println("touch");
    // 舵机开始转向90度位置，同时小灯以每秒5次闪烁；
    for (auto i = 0; i < 5; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
      ledcWrite(ledChannel, dutyCycle0 + (dutyCycle90 - dutyCycle0) / 5 * i);
    }

    digitalWrite(ledPin, HIGH);
    delay(5000);

    // 舵机臂开始回到0度位置，同时小灯以每秒5次闪烁；
    for (auto i = 0; i < 5; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
      ledcWrite(ledChannel, dutyCycle90 - (dutyCycle90 - dutyCycle0) / 5 * i);
    }

    digitalWrite(ledPin, LOW);
  }
}