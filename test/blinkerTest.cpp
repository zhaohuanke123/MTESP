#define BLINKER_WIFI
#include <Blinker.h>
char auth[] = "1ae364051dc8"; // �豸��Կ
char ssid[] = "0d00"; // WIFI�ȵ�
char pswd[] = "0d000721"; // WIFI����
#define LED_PIN 2 // LED����
// �½��������
BlinkerButton Button1("btn-abc");
BlinkerNumber Number1("num-abc");
int counter = 0;
// ���°�������ִ�иú���
void button1_callback(const String& state)
{
  BLINKER_LOG("get button state: ", state);
  digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // ��תLED��״̬
}
// ���δ�󶨵���������������ִ����������
void dataRead(const String& data)
{
  BLINKER_LOG("Blinker readString: ", data);
  counter++;
  Number1.print(counter);
}

void setup()
{
  // ��ʼ������
  Serial.begin(115200);
  BLINKER_DEBUG.stream(Serial);
  BLINKER_DEBUG.debugAll();
  // ��ʼ����LED��IO
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  // ��ʼ��blinker
  Blinker.begin(auth, ssid, pswd);
  Blinker.attachData(dataRead);
  Button1.attach(button1_callback);
}
void loop() {
  Blinker.run();
}
