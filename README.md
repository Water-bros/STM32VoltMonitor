# STM32VoltMonitor
2024年南方医科大学风标杯设计作品固件源码 创建于2024-12-01
<br>
### 功能清单
- [x] 测量负载电压
- [x] 设置保护电压和滞回电压
- [x] 在8段数码管上显示电压
- [x] 串口调试输出<br>
~~存储设置值到Flash中~~
- [ ] 数码管彩蛋

### 使用须知
1. 在release中下载固件，用ST-LINK Utility烧录
2. 若想修改源码自行编译，先在PlatformIO中新建项目，型号选择STM32F103C8，平台选择Arduino，项目创建完成后将整个libdeps目录替换掉本地.pio目录中同名文件夹，再将本项目src/main.cpp，platformio.ini放入本地对应文件夹中
3. 本代码固件已经过10余组作品测试，固件本身并无问题
4. 有疑问请在issue中提出
---
2024-12-07 更新<br>
上传正式代码以及固件，已完善所有已知问题<br>
ADC 测量值转换实际值经过多次线性拟合得到较为精确的结果（满足误差1%）<br><br>

2024-12-03 更新<br>
完赛！（上电测试前最后一秒调出来了谁懂<br><br>

2024-12-02 晚 更新<br>
终于烧录成功了，慢慢调代码……<br><br>

2024-12-02 更新<br>
代码应该没有问题，因为烧录不进自己画的板子里……<br>
大概率是赛事主办提供的原理图有问题，就此作罢了
