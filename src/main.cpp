/*************************************************

Copyright: Southern Medical University

Author: Water_bros Zhou & MH

Start Time: 2024-11-29 13:30
Update Time: 2024-12-07 13:00

Description: 2024 南方医科大学·风标杯 大二组题目设计作品源码

Thanks for using this code!
Thanks for your support!

**************************************************/

#include <Arduino.h>
#include <TM1652.h>
// #include <Flash.h> // Flash 读写的自定义库出问题了，暂时不用

#define RELAY_PIN PA1       // 继电器引脚
#define ADC_PIN PA2         // ADC引脚
#define KEY_SET_PIN PA3     // 设置键引脚
#define KEY_UP_PIN PA4      // 增加键引脚 与原理图不一致，注意
#define KEY_DOWN_PIN PA5    // 减少键引脚 与原理图不一致，注意
#define LED_PIN PA7         // 8段数码管引脚
#define SERIAL_TXD_PIN PA9  // 串口引脚
#define SERIAL_RXD_PIN PA10 // 串口引脚

struct RelayStruct // 继电器结构体
{
  int pin;
  bool state = 0;
  void init() // 初始化
  {
    pinMode(pin, OUTPUT);
    off();
  }
  void on() // 打开继电器输出
  {
    if (!state)
    {
      digitalWrite(pin, HIGH);
      state = 1;
    }
  };
  void off() // 关闭继电器输出
  {
    if (state)
    {
      digitalWrite(pin, LOW);
      state = 0;
    }
  };
} relay;

TM1652 led(LED_PIN); // 8段数码管对象

HardwareSerial Serial1(SERIAL_TXD_PIN, SERIAL_RXD_PIN); // 串口对象

double voltage = 0;                                               // 实时电压值
int protectVoltage = 150, hysteresisVoltage = 50, restoreVoltage; // 默认保护电压15V，滞回电压5V，恢复电压
int setMode = 3;                                                  // 0:设置保护电压，1:设置滞回电压，2:保存退出
int lastTime = 0;                                                 // 时间间隔获取的上一次毫秒数
int temp = 0, t = 0;

// u32 *voltageArray = (u32 *)malloc(2 * sizeof(u32));                 // 电压数组，用于存储在 Flash 中

bool addKeyEvent = false; // 按键事件标志
bool minusKeyEvent = false;

const char *textSET = "SET"; // 字母文本
const char *textHLL = "HLL";
const char *textYES = "YES";

void digitalLEDInit();               // 数码管初始化
void relayInit();                    // 继电器初始化
void keyInit();                      // 按键初始化
void ADCInit();                      // ADC初始化
void serialInit();                   // 串口初始化
int getInterval();                   // 获取时间间隔
void readVoltage();                  // 读取实时电压
void displayChar(const char *value); // 数码管显示字符
void displayVoltage(int v);          // 显示电压

// void readVoltageData();                    // 读取 Flash 中的电压数据
// void saveVoltageData();                    // 保存电压数据到 Flash

void displayCurrentVoltage(); // 显示当前电压
void modeLoop();              // 设置模式
void keySet();                // 设置键功能
void add();                   // 已废弃
void minus();                 // 已废弃
void keyAdd();                // 增加键功能
void keyMinus();              // 减少键功能
void relayControl();          // 继电器控制
void debugPrint();            // 调试输出

void digitalLEDInit()
{
  led.begin();
  displayCurrentVoltage();
}

void relayInit()
{
  relay.pin = RELAY_PIN;
  relay.init();
}

void keyInit()
{
  pinMode(KEY_SET_PIN, INPUT_PULLUP);
  pinMode(KEY_UP_PIN, INPUT_PULLUP);
  pinMode(KEY_DOWN_PIN, INPUT_PULLUP);

  attachInterrupt(KEY_SET_PIN, keySet, FALLING); // 绑定对应按键中断操作调用的函数
  attachInterrupt(KEY_UP_PIN, keyAdd, FALLING);
  attachInterrupt(KEY_DOWN_PIN, keyMinus, FALLING);
}

void ADCInit()
{
  pinMode(ADC_PIN, INPUT_ANALOG);
}

void serialInit()
{
  Serial1.begin(115200);
}

int getInterval() // 获取时间间隔
{
  int now = millis();
  int interval = now - lastTime;
  lastTime = now;
  return interval;
}

void readVoltage()
{
  int adcValue = analogRead(ADC_PIN);
  voltage = adcValue;
  // 线性回归方程，实际作近似处理
  // f(x) = 0.22772878710267208 * x + 2.968458045774822
  voltage = (double)adcValue * 0.2277 + 2.9685; // ADC测量值转换为电压值
}

void displayChar(const char *value)
{
  led.clearDisplay();
  led.sendAsciiChar(0, value[0], 0);
  led.sendAsciiChar(1, value[1], 0);
  led.sendAsciiChar(2, value[2], 0);
}

void displayVoltage(int v)
{
  int v1, v2, v3;
  v1 = v / 100;
  v2 = (v % 100) / 10;
  v3 = v % 10;
  led.clearDisplay();
  led.setDisplayDigit(v1, 0);
  led.setDisplayDigit(v2, 1, true);
  led.setDisplayDigit(v3, 2);
}

// void readVoltageData()
// {
//   flashRead(voltageArray);
//   protectVoltage = voltageArray[0];
//   hysteresisVoltage = voltageArray[1];
//   restoreVoltage = voltage + hysteresisVoltage;
// }

// void saveVoltageData()
// {
//   if (voltageArray[0] != protectVoltage || voltageArray[1] != hysteresisVoltage)
//   {
//     voltageArray[0] = protectVoltage;
//     voltageArray[1] = hysteresisVoltage;
//     flashWrite(voltageArray);
//   }
// }

void displayCurrentVoltage()
{
  // led.setDisplayToString("SOS");
  readVoltage();
  displayVoltage((int)voltage);
  delay(200);
  // debugPrint();
}

void keySet()
{
  t++;
  delay(100);
  modeLoop();
}

void modeLoop()
{
  if (getInterval() > 50)
  {
    if (t >= 1)
    {
      t = 0;
      setMode++;
    }
    switch (setMode % 4)
    {
    case 0: // 显示并设置保护电压
      protectVoltage += temp;
      temp = 0;
      displayChar(textSET);
      delay(400);
      displayVoltage(protectVoltage);
      delay(400);
      break;
    case 1: // 显示并设置滞回电压
      hysteresisVoltage += temp;
      temp = 0;
      displayChar(textHLL);
      delay(400);
      displayVoltage(hysteresisVoltage);
      delay(400);
      break;
    case 2: // 保存设置
      displayChar(textYES);
      delay(800);
      // saveVoltageData();
      setMode++;
    case 3:
      displayCurrentVoltage();
      // delay(200);
      break;
    default:
      displayChar("ERR");
      break;
    }
  }
}

void keyAdd()
{
  if (getInterval() > 80)
  {
    temp++;
  }
}

void keyMinus()
{
  if (getInterval() > 80)
  {
    temp--;
  }
}

void add() // 已废弃
{
  if (getInterval() > 50 && addKeyEvent)
  {
    switch (setMode)
    {
    case 0:
      // protectVoltageTemp++;
      // delay(100);
      displayVoltage(protectVoltage);
      break;
    case 1:
      // hysteresisVoltageTemp++;
      // delay(100);
      displayVoltage(hysteresisVoltage);
      break;
    }
  }
}

void minus() // 已废弃
{
  if (getInterval() > 50 && minusKeyEvent)
  {
    switch (setMode)
    {
    case 0:
      if (protectVoltage > 0)
      {
        protectVoltage--;
      }
      else
      {
        protectVoltage = 0;
      }
      // delay(100);
      displayVoltage(protectVoltage);
      break;
    case 1:
      if (hysteresisVoltage > 0)
      {
        hysteresisVoltage--;
      }
      else
      {
        hysteresisVoltage = 0;
      }
      // delay(100);
      displayVoltage(hysteresisVoltage);
      break;
    }
  }
}

void relayControl()
{
  readVoltage();
  if (voltage < protectVoltage) // 电压低于保护电压，断开继电器
  {
    relay.off();
  }
  else if (voltage > restoreVoltage) // 电压高于恢复电压，闭合继电器
  {
    relay.on();
  }
}

void debugPrint()
{
  Serial1.print("ProtectVoltage: ");
  Serial1.print((float)protectVoltage / 10.0);
  Serial1.println("V");
  Serial1.print("HysteresisVoltage: ");
  Serial1.print((float)hysteresisVoltage / 10.0);
  Serial1.println("V");
  Serial1.print("Voltage: ");
  Serial1.print((float)voltage / 10.0);
  Serial1.println("V");
  Serial1.println("===============");
};

void setup()
{
  relayInit();
  ADCInit();
  keyInit();
  digitalLEDInit();
  serialInit();
  // readVoltageData();
}

void loop()
{
  modeLoop();
  // keyAdd();
  // keyMinus();
  relayControl();
}
