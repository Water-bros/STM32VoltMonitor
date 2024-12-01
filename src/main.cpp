/*************************************************

Copyright: Southern Medical University

Author: Water_bros Zhou

Date: 2024-12-01

Description: 2024年南方医科大学风标杯设计作品源码

**************************************************/

#include <Arduino.h>
#include <TM1652.h>
// #include <Flash.h> // Flash 读写的自定义库出问题了，暂时不用

#define RELAY_PIN PA1    // 继电器引脚
#define ADC_PIN PA2      // ADC引脚
#define KEY_SET_PIN PA3  // 设置键引脚
#define KEY_UP_PIN PA4   // 增加键引脚 与原理图不一致，注意
#define KEY_DOWN_PIN PA5 // 减少键引脚 与原理图不一致，注意
#define LED_PIN PA7      // 8段数码管引脚

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

float voltage = 0;                                                  // 实时电压值
int protectVoltage = 3000, hysteresisVoltage = 600, restoreVoltage; // 默认保护电压30V，滞回电压6V，恢复电压
int setMode = 3;                                                    // 0:设置保护电压，1:设置滞回电压，2:保存退出
// u32 *voltageArray = (u32 *)malloc(2 * sizeof(u32));                 // 电压数组，用于存储在 Flash 中
const char *textSET = "SET";                                        // 字母文本
const char *textHLL = "HLL";
const char *textYES = "YES";

void digitalLEDInit();                     // 数码管初始化
void relayInit();                          // 继电器初始化
void keyInit();                            // 按键初始化
void ADCInit();                            // ADC初始化
void displayDigits(int value, int dotPos); // 数码管数字显示
void displayChar(const char *value);       // 数码管显示字符
// void readVoltageData();                    // 读取 Flash 中的电压数据
// void saveVoltageData();                    // 保存电压数据到 Flash
void displayCurrentVoltage();              // 显示当前电压
void displayVoltage(int v);                // 显示电压
void keySetMode();                         // 设置模式
void keySet();                             // 设置键功能
void keyAdd();                             // 增加键功能
void keyMinus();                           // 减少键功能
void relayControl();                       // 继电器控制

void digitalLEDInit()
{
  led.begin();
  delay(100);
  displayCurrentVoltage();
}

void relayInit()
{
  relay.pin = RELAY_PIN;
  relay.init();
}

void keyInit()
{
  pinMode(KEY_SET_PIN, INPUT);
  pinMode(KEY_UP_PIN, INPUT);
  pinMode(KEY_DOWN_PIN, INPUT);

  attachInterrupt(KEY_SET_PIN, keySetMode, FALLING); // 绑定对应按键中断操作调用的函数
  attachInterrupt(KEY_UP_PIN, keyAdd, FALLING);
  attachInterrupt(KEY_DOWN_PIN, keyMinus, FALLING);
}

void ADCInit()
{
  pinMode(ADC_PIN, INPUT_ANALOG);
}

void displayDigits(int digit, int dotPos)
{
  int d1, d2, d3;
  d1 = digit / 100;
  d2 = (digit % 100) / 10;
  d3 = digit % 10;
  led.clearDisplay();

  switch (dotPos)
  {
  case 1:
    led.setDisplayDigit(d1, 1);
    led.setDisplayDigit(d2, 0);
    led.setDisplayDigit(d3, 0);
    break;
  case 2:
    led.setDisplayDigit(d1, 0);
    led.setDisplayDigit(d2, 1);
    led.setDisplayDigit(d3, 0);
    break;
  };
}

void displayChar(const char *value)
{
  led.clearDisplay();
  led.setDisplayToString(value);
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

void readVoltage()
{
  int adcValue = analogRead(ADC_PIN);
  voltage = (float)adcValue / 4095.0 * 5460;
}

void displayCurrentVoltage()
{
  if (setMode == 3)
  {
    delay(50);
    readVoltage();
    displayVoltage((int)voltage);
  }
}

void displayVoltage(int v)
{
  delay(20);

  if (v < 1000)
  {
    displayDigits(v, 1);
  }
  else
  {
    displayDigits(v / 10, 2);
  }
}

void keySetMode()
{
  delay(20); // 延时去抖动
  setMode = (setMode + 1) % 4;
  keySet();
}

void keySet()
{
  switch (setMode)
  {
  case 0: // 显示并设置保护电压
    displayChar(textSET);
    delay(400);
    displayVoltage(protectVoltage);
    delay(400);
    break;
  case 1: // 显示并设置滞回电压
    displayChar(textHLL);
    delay(400);
    displayVoltage(hysteresisVoltage);
    delay(400);
    break;
  case 2: // 保存设置
    displayChar(textYES);
    delay(1000);
    // saveVoltageData();
    setMode++;
    break;
  default:
    break;
  }
}

void keyAdd()
{
  switch (setMode)
  {
  case 0:
    protectVoltage++;
    break;
  case 1:
    hysteresisVoltage++;
    break;
  default:
    break;
  }
}

void keyMinus()
{
  switch (setMode)
  {
  case 0:
    protectVoltage--;
    break;
  case 1:
    hysteresisVoltage--;
    break;
  default:
    break;
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

void setup()
{
  relayInit();
  ADCInit();
  keyInit();
  digitalLEDInit();
  // readVoltageData();
  delay(20);
}

void loop()
{
  displayCurrentVoltage();
  keySet();
  relayControl();
}
