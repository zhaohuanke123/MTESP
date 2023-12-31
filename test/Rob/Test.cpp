#define ENCODER_L 12 //8号引脚
#define DIRECTION_L 13 //9号引脚
#define ENCODER_R 25 //3号引脚
#define DIRECTION_R 26  //2号引脚
#define interrupt_time 20 // 中断时间
#define L_DIR 14   //7号引脚
#define L_SPEED 16  //5号引脚
#define R_DIR 17   //4号引脚
#define R_SPEED 27  //6号引脚

int TARGET_L_SET = 1000;
int TARGET_R_SET = 1000;
int TARGET_L = TARGET_L_SET;
int TARGET_R = TARGET_R_SET;
int TARGET_L_OFFSET = 0;
int TARGET_R_OFFSET = 0;
int Max_L_Speed = TARGET_L_SET;
int Max_R_Speed = TARGET_R_SET;

//ALPHA（建议在100-500之间）值越小和MAXINC（建议不越过10）越大，速度调节越快，太快容易转速调节过头
#define ALPHA 200.0
#define MAXINC 8.0
//轮子启动初始量，使轮子快束起动
#define BOOST 80.0

#include <Arduino.h>
#include<Ticker.h>
#include <WiFi.h>  //wifi功能需要的库
Ticker timer_read_encoder; // 中断函数定时器定义

int32_t Velocity_L, Velocity_R; //左右轮编码器数据
int16_t Velocity_Left, Velocity_Right; //左右轮速度

void READ_ENCODER_L(void);
void READ_ENCODER_R(void);
void read(void);

void READ_ENCODER_L(void) {
    if (digitalRead(ENCODER_L) == LOW) {
        //如果是下降沿触发的中断
        if (digitalRead(DIRECTION_L) == LOW)
            Velocity_L--;
        //根据另外一相电平判定方向
        else Velocity_L++;
    }
    else { //如果是上升沿触发的中断
        if (digitalRead(DIRECTION_L) == LOW)
            Velocity_L++; //根据另外一相电平判定方向
        else Velocity_L--;
    }
}

void READ_ENCODER_R(void) {
    if (digitalRead(ENCODER_R) == LOW) { //如果是下降沿触发的中断
        if (digitalRead(DIRECTION_R) == LOW)
            Velocity_R--;//根据另外一相电平判定方向
        else Velocity_R++;
    }
    else { //如果是上升沿触发的中断
        if (digitalRead(DIRECTION_R) == LOW) Velocity_R++; //根据另外一相电平判定方向
        else Velocity_R--;
    }
}

void read(void) {
    Velocity_Left = Velocity_L * 2000 / interrupt_time;
    Velocity_L = 0; //读取左轮编码器数据，并清零，这就是通过M法测速（单位时间内的脉冲数）得到速度。
    Velocity_Right = Velocity_R * 2000 / interrupt_time;
    Velocity_R = 0; //读取右轮编码器数据，并清零
}

float l_speed = 0, r_speed = 0;

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

// 几个变量，用于表示前进回退，停止（1，-1，0），左右（1，-1）
int  forward = 0, turn = 0;

void taskOne(void* pvParameters)
{
    int i;
    l_speed = 0; r_speed = 0;
    while (1) {
        if (!forward) {
            analogWrite(L_SPEED, 0);
            analogWrite(R_SPEED, 0);
        }
        else if (turn == 1) {
            digitalWrite(L_DIR, 0);
            digitalWrite(R_DIR, 1);
            analogWrite(L_SPEED, 0);
            analogWrite(R_SPEED, r_speed);
            if (l_speed == 0) {
                l_speed = BOOST;
                r_speed = BOOST;
            }

            // if (Velocity_Left > -(TARGET_L - 50))
                // l_speed = l_speed + ((Velocity_Left + TARGET_L) / ALPHA > MAXINC ? MAXINC : (Velocity_Left + TARGET_L) / ALPHA);
            // else if (Velocity_Left < -(TARGET_L + 50))
                // l_speed = l_speed - ((-TARGET_L - Velocity_Left) / ALPHA > MAXINC ? MAXINC : (-TARGET_L - Velocity_Left) / ALPHA);

            if (Velocity_Right < (TARGET_R - 50))
                r_speed = r_speed + ((TARGET_R - Velocity_Right) / ALPHA > MAXINC ? MAXINC : (TARGET_R - Velocity_Right) / ALPHA);
            else if (Velocity_Right > (TARGET_R + 50))
                r_speed = r_speed - ((Velocity_Right - TARGET_R) / ALPHA > MAXINC ? MAXINC : (Velocity_Right - TARGET_R) / ALPHA);

            // if (l_speed > 255)
                // l_speed = 255;
            if (r_speed > 255)
                r_speed = 255;
        }
        else if (turn == -1) {

            digitalWrite(L_DIR, 1);
            digitalWrite(R_DIR, 0);
            analogWrite(L_SPEED, l_speed);
            analogWrite(R_SPEED, 0);
            if (l_speed == 0) {
                l_speed = BOOST;
                r_speed = BOOST;
            }

            if (Velocity_Left < (TARGET_L - 50))
                l_speed = l_speed + ((TARGET_L - Velocity_Left) / ALPHA > MAXINC ? MAXINC : (TARGET_L - Velocity_Left) / ALPHA);
            else if (Velocity_Left > (TARGET_L + 50))
                l_speed = l_speed - ((Velocity_Left - TARGET_L) / ALPHA > MAXINC ? MAXINC : (Velocity_Left - TARGET_L) / ALPHA);

            // if (Velocity_Right > -(TARGET_R - 50))
                // r_speed = r_speed + ((Velocity_Right + TARGET_R) / ALPHA > MAXINC ? MAXINC : (Velocity_Right + TARGET_R) / ALPHA);
            // else if (Velocity_Right < -(TARGET_R + 50))
                // r_speed = r_speed - ((-TARGET_R - Velocity_Right) / ALPHA > MAXINC ? MAXINC : (-TARGET_R - Velocity_Right) / ALPHA);

            if (l_speed > 255)
                l_speed = 255;

            // if (r_speed > 255)
                // r_speed = 255;
        }

        else  if (forward == 1) {

            digitalWrite(L_DIR, 1);
            digitalWrite(R_DIR, 1);
            analogWrite(L_SPEED, l_speed);
            analogWrite(R_SPEED, r_speed);

            if (l_speed == 0) {
                l_speed = BOOST;
                r_speed = BOOST;
            }

            if (Velocity_Left < (TARGET_L - 50))
                l_speed = l_speed + ((TARGET_L - Velocity_Left) / ALPHA > MAXINC ? MAXINC : (TARGET_L - Velocity_Left) / ALPHA);
            else if (Velocity_Left > (TARGET_L + 50))
                l_speed = l_speed - ((Velocity_Left - TARGET_L) / ALPHA > MAXINC ? MAXINC : (Velocity_Left - TARGET_L) / ALPHA);

            if (Velocity_Right < (TARGET_R - 50))
                r_speed = r_speed + ((TARGET_R - Velocity_Right) / ALPHA > MAXINC ? MAXINC : (TARGET_R - Velocity_Right) / ALPHA);
            else if (Velocity_Right > (TARGET_R + 50))
                r_speed = r_speed - ((Velocity_Right - TARGET_R) / ALPHA > MAXINC ? MAXINC : (Velocity_Right - TARGET_R) / ALPHA);

            if (l_speed > 255)
                l_speed = 255;
            if (r_speed > 255)
                r_speed = 255;

        }
        else if (forward == -1) {

            digitalWrite(L_DIR, 0);
            digitalWrite(R_DIR, 0);
            analogWrite(L_SPEED, l_speed);
            analogWrite(R_SPEED, r_speed);

            if (l_speed == 0) {
                l_speed = BOOST;
                r_speed = BOOST;
            }

            if (Velocity_Left > -(TARGET_L - 50))
                l_speed = l_speed + ((Velocity_Left + TARGET_L) / ALPHA > MAXINC ? MAXINC : (Velocity_Left + TARGET_L) / ALPHA);
            else if (Velocity_Left < -(TARGET_L + 50))
                l_speed = l_speed - ((-TARGET_L - Velocity_Left) / ALPHA > MAXINC ? MAXINC : (-TARGET_L - Velocity_Left) / ALPHA);

            if (Velocity_Right > -(TARGET_R - 50))
                r_speed = r_speed + ((Velocity_Right + TARGET_R) / ALPHA > MAXINC ? MAXINC : (Velocity_Right + TARGET_R) / ALPHA);
            else if (Velocity_Right < -(TARGET_R + 50))
                r_speed = r_speed - ((-TARGET_R - Velocity_Right) / ALPHA > MAXINC ? MAXINC : (-TARGET_R - Velocity_Right) / ALPHA);

            if (l_speed > 255)
                l_speed = 255;

            if (r_speed > 255)
                r_speed = 255;
        }
        delay(50);
        // l_speed = 0; r_speed = 0;
        // for (i = 0; i < 80; i++) {
        //     digitalWrite(L_DIR, 0);
        //     digitalWrite(R_DIR, 1);
        //     analogWrite(L_SPEED, l_speed);
        //     analogWrite(R_SPEED, r_speed);
        //     if (i == 10) { l_speed = BOOST; r_speed = BOOST; }
        //     else if (i > 10)
        //     {
        //         if (Velocity_Left > -(TARGET_L - 50))
        //             l_speed = l_speed + ((Velocity_Left + TARGET_L) / ALPHA > MAXINC ? MAXINC : (Velocity_Left + TARGET_L) / ALPHA);
        //         else if (Velocity_Left < -(TARGET_L + 50))
        //             l_speed = l_speed - ((-TARGET_L - Velocity_Left) / ALPHA > MAXINC ? MAXINC : (-TARGET_L - Velocity_Left) / ALPHA);

        //         if (Velocity_Right < (TARGET_R - 50))
        //             r_speed = r_speed + ((TARGET_R - Velocity_Right) / ALPHA > MAXINC ? MAXINC : (TARGET_R - Velocity_Right) / ALPHA);
        //         else if (Velocity_Right > (TARGET_R + 50))
        //             r_speed = r_speed - ((Velocity_Right - TARGET_R) / ALPHA > MAXINC ? MAXINC : (Velocity_Right - TARGET_R) / ALPHA);

        //         if (l_speed > 255)
        //             l_speed = 255;

        //         if (r_speed > 255)
        //             r_speed = 255;
        //     }
        //     delay(50);
        // }

        // l_speed = 0; r_speed = 0;
        // for (i = 0; i < 80; i++) {
        //     digitalWrite(L_DIR, 1);
        //     digitalWrite(R_DIR, 0);
        //     analogWrite(L_SPEED, l_speed);
        //     analogWrite(R_SPEED, r_speed);
        //     if (i == 10) { l_speed = BOOST; r_speed = BOOST; }
        //     else if (i > 10)
        //     {
        //         if (Velocity_Left < (TARGET_L - 50))
        //             l_speed = l_speed + ((TARGET_L - Velocity_Left) / ALPHA > MAXINC ? MAXINC : (TARGET_L - Velocity_Left) / ALPHA);
        //         else if (Velocity_Left > (TARGET_L + 50))
        //             l_speed = l_speed - ((Velocity_Left - TARGET_L) / ALPHA > MAXINC ? MAXINC : (Velocity_Left - TARGET_L) / ALPHA);

        //         if (Velocity_Right > -(TARGET_R - 50))
        //             r_speed = r_speed + ((Velocity_Right + TARGET_R) / ALPHA > MAXINC ? MAXINC : (Velocity_Right + TARGET_R) / ALPHA);
        //         else if (Velocity_Right < -(TARGET_R + 50))
        //             r_speed = r_speed - ((-TARGET_R - Velocity_Right) / ALPHA > MAXINC ? MAXINC : (-TARGET_R - Velocity_Right) / ALPHA);

        //         if (l_speed > 255)
        //             l_speed = 255;
        //         if (r_speed > 255)
        //             r_speed = 255;
        //     }
        //     delay(50);
        // }

    }
}


WiFiUDP Udp;//声明UDP对象
uint16_t udp_port = 1122;  //存储需要监听的端口号
const char* wifi_SSID = "ZHK_Udp";  //存储AP的名称信息
const char* wifi_Password = "ZHK_1234";  //存储AP的密码信息

void setup() {
    Serial.begin(115200);
    pinMode(ENCODER_L, INPUT); //编码器引脚 输入模式
    pinMode(ENCODER_R, INPUT); //编码器引脚 输入模式
    pinMode(DIRECTION_L, INPUT); //编码器引脚 输入模式
    pinMode(DIRECTION_R, INPUT); //编码器引脚 输入模式
    pinMode(L_DIR, OUTPUT);
    pinMode(L_SPEED, OUTPUT);
    pinMode(R_DIR, OUTPUT);
    pinMode(R_SPEED, OUTPUT);
    //编码器接口1 开启外部跳边沿中断
    attachInterrupt(ENCODER_L, READ_ENCODER_L, CHANGE);
    //中断函数READ_ENCODER_L
    //编码器接口2 开启外部跳边沿中断
    attachInterrupt(ENCODER_R, READ_ENCODER_R, CHANGE);
    //中断函数READ_ENCODER_R
    interrupts();
    //打开外部中断
    timer_read_encoder.attach_ms(interrupt_time, read);

    xTaskCreatePinnedToCore(
      taskOne, /* Task function. */
      "TaskOne", /* String with name of task. */
      1000, /* Stack size in bytes. */
      NULL, /* Parameter passed as input of the task */
      1, /* Priority of the task. */
      NULL, /* Task handle. */
      1);

    WiFi.softAP(wifi_SSID, wifi_Password);  //打开ESP32热点
    Udp.begin(udp_port);//启动UDP监听这个端口
}


#define CENTERX 50
#define CENTERY 50
char incomingPacket[255];  //存储Udp客户端发过来的数据

uint8_t m_xAxis[3] = { CENTERX,CENTERX,CENTERX }, m_yAxis[3] = { CENTERY,CENTERY,CENTERY }; //用于保存两个摇杆的回传数据，0下标没有使用,1是左边摇杆数据，2是右边摇杆数据
int getSpeed2(float distance, int xAxis) //差速时，计算其中一个轮的转速量
{
    return distance * (CENTERX - fabs(xAxis - CENTERX)) * (CENTERX - fabs(xAxis - CENTERX)) / (CENTERX * CENTERX); //使用曲线映射
}
void Joystick_handle(int dev, uint8_t xAxis, uint8_t yAxis)
{
    m_xAxis[dev] = xAxis; m_yAxis[dev] = yAxis;
    if (m_xAxis[1] == CENTERX) //归位停止,（恢复方向？） 
    {
        forward = 0;
        turn = 0;
        return;
    }
    else
    {
        float distance = fabs(m_yAxis[1] - CENTERY);

        if (m_yAxis[1] < CENTERY) //摇杆向上 
        {
            forward = 1;
            TARGET_L = distance * Max_L_Speed / CENTERY;
            TARGET_R = distance * Max_R_Speed / CENTERY;
            if (m_xAxis[2] < CENTERX)
            {
                turn = 1;
            }
            else if (m_xAxis[2] > CENTERX)
            {
                turn = -1;
            }
            else
            {
                turn = 0;
            }
        }
        else //后退 
        {
            forward = -1;
            TARGET_L = distance * Max_L_Speed / CENTERY;
            TARGET_R = distance * Max_R_Speed / CENTERY;
            if (m_xAxis[2] < CENTERX)
            {
                turn = 1;
            }
            else if (m_xAxis[2] > CENTERX)
            {
                turn = -1;
            }
            else
            {
                turn = 0;
            }
        }
    }
}

void loop() {
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
        Split(incomingPacket);
        if (incomingPacket[0] == '1' || incomingPacket[0] == '2')
        {//处理摇杆数据
            Joystick_handle(str2num(list[0]), str2num(list[1]), str2num(list[2]));
        }
        else
        {
            if (incomingPacket[0] == '3') {
                TARGET_L_OFFSET = str2num(list[1]);
                Max_L_Speed = TARGET_L_SET + TARGET_L_OFFSET;
            }
            else if (incomingPacket[0] == '4') {
                TARGET_R_OFFSET = str2num(list[1]);
                Max_R_Speed = TARGET_R_SET + TARGET_R_OFFSET;
            }
            else if (incomingPacket[0] == '5') {
                TARGET_L_SET = str2num(list[1]);
                Max_L_Speed = TARGET_L_SET + TARGET_L_OFFSET;
                TARGET_R_SET = str2num(list[1]);
                Max_R_Speed = TARGET_R_SET + TARGET_R_OFFSET;
            }
        }
    }
}