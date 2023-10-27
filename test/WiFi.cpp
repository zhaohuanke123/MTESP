// 本实验需要两块ESP32板合作完成！！
// （1）一块ESP32连接超声波模块，读取测距数据，板上连接IO12口接LED小灯根据距离的
// 远近以不同的周期闪动。
// 闪动周期（毫秒）= 990*测距数据（厘米） *测距数据（厘米） /(35*35)+10（当测距数据
// 小于35时）
// 当测距数据大于35时，小灯灭，不闪动。
// 同时，通过WIFI UDP通讯发送给另一块ESP32。
// （2）另一块ESP32将得到的测距数据显示在串口监视器中，同时控制板载小灯（IO02）根
// 据接收到的距离的远近以不同的周期闪动，周期与第一块ESP32上的小灯相同。

#include <Arduino.h>
#include <Ticker.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

const char* ssid = "zhk_wifi";
const char* password = "1234567890";
IPAddress  udpAddress(192, 168, 4, 1);
uint16_t udp_port = 1122;

const int ledPin = 2;
int distance;

void led() {
  static int count = 0;

  if (distance > 35 || distance <= 0)
  {
    digitalWrite(ledPin, LOW);
  }
  else
    if (count > (990 * distance * distance / (35 * 35) + 10)) {
      digitalWrite(ledPin, !digitalRead(ledPin));
      count = 0;
    }
    else {
      count++;
    }
}


// 发送端
#define __server__
#ifdef __server__

WiFiUDP udp;

const int trigPin = 26;
const int echoPin = 25;

boolean connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_AP_STACONNECTED:
    connected = true;
    break;
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    connected = false;
    break;
  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, password);
  udp.begin(udp_port);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Ticker* ticker = new Ticker();
  ticker->attach(0.001, led);

  Ticker* udpTicker = new Ticker();
  udpTicker->attach(0.2, []() {
    if (WiFi.status() == WL_CONNECTED)
    {
      static int oldDistance = 0;
      if (distance == oldDistance) {
        return;
      }
      oldDistance = distance;
      udp.beginPacket(udpAddress, udp_port);
      udp.print(String(distance));
      udp.endPacket();
      Serial.println("Send Data" + String(distance));
    }
  });
}

void loop()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long duration = pulseIn(echoPin, HIGH);
  if (duration != 0)
    distance = (float)duration / 58;

  Serial.print("Distance: ");
  Serial.println(distance);
}
#endif

// 接收端
#ifndef __server__
WiFiUDP udp;

Ticker ticker;

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  udp.begin(udp_port);

  pinMode(ledPin, OUTPUT);

  ticker.attach(0.001, led);
}

void loop()
{
  if (WiFi.softAPgetStationNum() == 0)
  {
    distance = 0;
  }
  else {
    int Data_length = udp.parsePacket();
    if (Data_length)
    {
      String udpMsg = udp.readString();
      distance = udpMsg.toFloat();
    }
  }
}
#endif