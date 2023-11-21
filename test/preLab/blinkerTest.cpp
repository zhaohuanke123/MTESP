#define BLINKER_WIFI
#include <Blinker.h>

char auth[] = "1ae364051dc8"; // 设备密钥
char ssid[] = "0d00"; // WIFI热点
char pswd[] = "0d000721"; // WIFI密码

const int ledPin = 2;
const int ledChannel = 0;

// 新建组件对象
BlinkerButton Button1("btn-abc");
// BlinkerNumber Number1("num-abc");
BlinkerSlider Slider1("ran-ufx");

bool ledState = false;
int ledLevel = 0;

// 按下按键即会执行该函数
void button1_callback(const String& state)
{
  BLINKER_LOG("get button state: ", state);
  ledState = !ledState;

  if (ledState) {
    // digitalWrite(ledPin, LOW);
    ledcWrite(ledChannel, ledLevel);
  }
  else {
    // digitalWrite(ledPin, HIGH);
    ledcWrite(ledChannel, 0);
  }
}
void slider1_callback(int32_t data)
{
  BLINKER_LOG("get slider state: ", data);
  // 0到100 对应到 0 到 255
  ledLevel = data * 255 / 100;
  if (ledState)
    ledcWrite(ledChannel, ledLevel);
}

void setup()
{
  // 初始化串口
  Serial.begin(115200);
  BLINKER_DEBUG.stream(Serial);
  BLINKER_DEBUG.debugAll();
  // 初始化有LED的IO
  ledcSetup(0, 5000, 8);
  ledcAttachPin(ledPin, ledChannel);

  // 初始化blinker
  Blinker.begin(auth, ssid, pswd);
  // Blinker.attachData(dataRead);
  Button1.attach(button1_callback);
  Slider1.attach(slider1_callback);

}
void loop() {
  Blinker.run();
}
