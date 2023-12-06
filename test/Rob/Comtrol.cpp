#include <WiFi.h>  //wifi功能需要的库
#include <Ticker.h>
#define ENCODER_L 12 //8号引脚
#define DIRECTION_L 13 //9号引脚
#define ENCODER_R 25 //3号引脚
#define DIRECTION_R 26  //2号引脚
#define interrupt_time 20 // 中断时间
#define LSPEED 16 // 左轮速度引脚 
#define LDIREC 14 // 左轮方向引脚 
#define RSPEED 27 // 右轮速度引脚 
#define RDIREC 17 // 右轮方向引脚 
//PWM 参数 
int channel_L = 0; //左轮 PWM 通道 
int channel_R = 1; //右轮 PWM 通道 
int freq = 500; // PWM 频率 
int resolution_bits = 10;// 0~1023 
int PWM_L_SET = 600;
int PWM_R_SET = 600;
int PWM_L = 600; //左轮最大量1023，需要根据自己的机器进行调整
int PWM_R = 600; //右轮最大量1023，需要根据自己的机器进行调整 
int PML_L_OFFSET = 0;
int PML_R_OFFSET = 0;

WiFiUDP Udp;//声明UDP对象

const char* wifi_SSID = "123456";  //存储AP的名称信息
const char* wifi_Password = "12345678";  //存储AP的密码信息

uint16_t udp_port = 1122;  //存储需要监听的端口号

char incomingPacket[255];  //存储Udp客户端发过来的数据

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);  //开启串口，波特率为115200
    pinMode(LDIREC, OUTPUT);// 左轮方向引脚 
    pinMode(RDIREC, OUTPUT);// 右轮方向引脚 

    //设置向前 
    digitalWrite(LDIREC, HIGH);
    digitalWrite(RDIREC, HIGH);

    //设置 PWM 通道 并连接对应引脚 
    ledcSetup(channel_L, freq, resolution_bits); // 设置左轮 PWM 通道 
    ledcSetup(channel_R, freq, resolution_bits); // 设置右轮 PWM 通道 

    // 将通道与对应的引脚连接 
    ledcAttachPin(LSPEED, channel_L);
    ledcAttachPin(RSPEED, channel_R);

    WiFi.softAP(wifi_SSID, wifi_Password);  //打开ESP32热点
    Udp.begin(udp_port);//启动UDP监听这个端口
}

#define CENTERX 50
#define CENTERY 50

uint8_t m_xAxis[3] = { CENTERX,CENTERX,CENTERX }, m_yAxis[3] = { CENTERY,CENTERY,CENTERY }; //用于保存两个摇杆的回传数据，0下标没有使用,1是左边摇杆数据，2是右边摇杆数据
int getSpeed2(float distance, int xAxis) //差速时，计算其中一个轮的转速量
{
    return distance * (CENTERX - fabs(xAxis - CENTERX)) * (CENTERX - fabs(xAxis - CENTERX)) / (CENTERX * CENTERX); //使用曲线映射
}
//处理函数
void Joystick_handle(int dev, uint8_t xAxis, uint8_t yAxis)
{
    m_xAxis[dev] = xAxis; m_yAxis[dev] = yAxis;
    if (m_xAxis[1] == CENTERX && m_xAxis[1] == CENTERY) //归位停止,（恢复方向？） 
    {
        ledcWrite(channel_L, 0);
        ledcWrite(channel_R, 0);
        return;
    }
    else
    {
        float distance = fabs(m_yAxis[1] - CENTERY);

        if (m_yAxis[1] < CENTERY) //摇杆向上 
        {
            digitalWrite(LDIREC, HIGH);
            digitalWrite(RDIREC, HIGH);
            if (m_xAxis[2] < CENTERX)
            {
                ledcWrite(channel_L, getSpeed2(distance, m_xAxis[2]) * PWM_L / CENTERY);
                ledcWrite(channel_R, distance * PWM_R / CENTERY);
            }
            else
            {

                ledcWrite(channel_L, PWM_L * distance / CENTERY);
                ledcWrite(channel_R, getSpeed2(distance, m_xAxis[2]) * PWM_R / CENTERY);
            }
        }
        else //后退 
        {
            digitalWrite(LDIREC, LOW);
            digitalWrite(RDIREC, LOW);
            if (m_xAxis[2] < CENTERX)
            {
                ledcWrite(channel_L, getSpeed2(distance, m_xAxis[2]) * PWM_L / CENTERY);
                ledcWrite(channel_R, PWM_R * distance / CENTERY);
            }
            else
            {

                ledcWrite(channel_L, PWM_L * distance / CENTERY);
                ledcWrite(channel_R, getSpeed2(distance, m_xAxis[2]) * PWM_R / CENTERY);
            }
        }
    }
}

char* list[10];
void Split(char* s)
{
    list[0] = s;
    int i = 1;
    while (*s != 0)
    {
        if (*s == ',')
        {
            *s = 0;
            s++;
            list[i++] = s;
        }
        else s++;
    }
}

int str2num(char* s)
{
    int ret = 0;

    // 如果第一位是负号
    if (*s == '-')
    {
        s++;
        while (*s != 0)
            ret = ret * 10 - ((*s++) - '0');
        return ret;
    }


    while (*s != 0)
        ret = ret * 10 + ((*s++) - '0');
    return ret;
}

void loop() {
    // put your main code here, to run repeatedly:

    /*接收发送过来的Udp数据*/
    int Data_length = Udp.parsePacket();  //获取接收的数据的长度
    if (Data_length)  //如果有数据那么Data_length不为0，无数据Data_length为0
    {
        int len = Udp.read(incomingPacket, 255);  //读取数据，将数据保存在数组incomingPacket中
        if (len > 0)  //为了避免获取的数据后面乱码做的判断
        {
            incomingPacket[len] = 0;
        }

        //Serial.println(incomingPacket); //调试获得的值
        if (incomingPacket[0] == '1' || incomingPacket[0] == '2')
        {//处理摇杆数据
            Split(incomingPacket);
            Joystick_handle(str2num(list[0]), str2num(list[1]), str2num(list[2]));
        }
        else
        {
            Split(incomingPacket);
            if (incomingPacket[0] == '3') {
                PML_L_OFFSET = str2num(list[1]);
                PWM_L = PWM_L_SET + PML_L_OFFSET;
            }
            else if (incomingPacket[0] == '4') {
                PML_R_OFFSET = str2num(list[1]);
                PWM_R = PWM_R_SET + PML_R_OFFSET;
            }
            else if (incomingPacket[0] == '5') {
                PWM_L_SET = str2num(list[1]);
                PWM_L = PWM_L_SET + PML_L_OFFSET;
                PWM_R_SET = str2num(list[1]);
                PWM_R = PWM_R_SET + PML_R_OFFSET;
            }
        }
    }
}
