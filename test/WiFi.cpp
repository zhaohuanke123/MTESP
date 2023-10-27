// ��ʵ����Ҫ����ESP32�������ɣ���
// ��1��һ��ESP32���ӳ�����ģ�飬��ȡ������ݣ���������IO12�ڽ�LEDС�Ƹ��ݾ����
// Զ���Բ�ͬ������������
// �������ڣ����룩= 990*������ݣ����ף� *������ݣ����ף� /(35*35)+10�����������
// С��35ʱ��
// ��������ݴ���35ʱ��С���𣬲�������
// ͬʱ��ͨ��WIFI UDPͨѶ���͸���һ��ESP32��
// ��2����һ��ESP32���õ��Ĳ��������ʾ�ڴ��ڼ������У�ͬʱ���ư���С�ƣ�IO02����
// �ݽ��յ��ľ����Զ���Բ�ͬ�������������������һ��ESP32�ϵ�С����ͬ��

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


// ���Ͷ�
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

// ���ն�
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