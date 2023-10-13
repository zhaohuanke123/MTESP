//实现如下功能 ：
// （1）红灯点亮10秒；
// （2）红灯熄灭，黄灯以每秒一次的速度闪烁3次；
// （3）黄灯熄灭，绿灯点亮10秒；
// （4）绿灯熄灭，从（1）重复；
#include <Arduino.h>
#include <Ticker.h> 

#define LED_RED 26
#define LED_YELLOW 25
#define LED_GREEN 27

Ticker timer_red;
Ticker timer_yellow;
Ticker timer_green;

void timer_red_callback(void);
void timer_yellow_callback(void);
void timer_green_callback(void);

void timer_red_callback()
{
  static int count = 0;
  count++;

  if (count == 10)
  {
    count = 0;
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    timer_red.detach();
    timer_yellow.attach(0.5, timer_yellow_callback);
  }
}

void timer_yellow_callback()
{
  static int count = 0;
  count++;

  if (count < 6)
  {
    digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));
  }
  else if (count == 6)
  {
    count = 0;
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
    timer_yellow.detach();
    timer_green.attach(1, timer_green_callback);
  }
}

void timer_green_callback()
{
  static int count = 0;
    count++;

  if (count == 10)
  {
    count = 0;
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    timer_green.detach();
    timer_red.attach(1, timer_red_callback);
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_RED, HIGH);
  timer_red.attach(1, timer_red_callback);
}
void loop() {

}