/**********************************************************************
*          Project: DTU_demo Ver:1.2
*          Le_destin
*          2676131208@qq.com
*          Just for DTU module
*          Show Debug Message When ESP8266 Module Start Push Pin13 Down
***********************************************************************/
#include <ESP8266WiFi.h>             //wifi头文件
#include <PubSubClient.h>            //client头文件用于建立MQTT连接
#include <ArduinoJson.h>             //Json类型头文件
#include <ACROBOTIC_SSD1306.h>       //OLED驱动头文件
/*连接WiFi名称和账号密码可配置 easyconnect 一键配网*/
#define WIFI_SSID "Meizu215"
#define WIFI_PASSWD "12345687" 
//----------------------------------------------------------------------------------------------------------------
/*阿里云物联网连接三元组每个设备烧录，用于身份识别*/
#define PRODUCT_KEY "a1JOuF58qU1"
#define DEVICE_NAME "pyHC0jntHJ20EAeRrnav"
#define DEVICE_SECRET "qeR5g24SX0CgtKNxCNKc9axFGiCctrSi" 
/* 
 a1JOuF58qU1
 pyHC0jntHJ20EAeRrnav
 qeR5g24SX0CgtKNxCNKc9axFGiCctrSi
*/
/*线上环境域名和端口号用于连接阿里云服务*/
#define MQTT_SERVER PRODUCT_KEY".iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define MQTT_PORT 1883 
#define MQTT_USRNAME DEVICE_NAME "&" PRODUCT_KEY 
//身份加密
#define CLIENT_ID "test|securemode=3,timestamp=123456789,signmethod=hmacsha1|" 
#define MQTT_PASSWD "5a4289dde59da11021f230478bad75402f706311"
/*MQTT签名的信息要用哈希进行加密*/
// TODO: MQTT连接的签名信息，哈希加密请以"clientIdtestdeviceName"+设备名称+"productKey"+设备模型标识+“timestamp123456789”
// 前往http://tool.oschina.net/encrypt?type=2进行加密格式如下 可以软实现HASH加密
// HMACSHA1/_SRC/ clientIdtestdeviceNamehuman04productKeya1rezUVs103timestamp123456789
/*阿里云ALINK协议段定义*/
#define ALINK_BODY_FORMAT "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":%s,%s,%s,%s,%s}" // 参数部分宏 一次5个
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"               //固定
#define ALINK_TOPIC_PROP_POSTRSP "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post_reply"      //固定
#define ALINK_TOPIC_PROP_SET "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set"               //固定
#define ALINK_METHOD_PROP_POST "thing.event.property.post"                                                   //固定
//---------------------------------------------------------------------------------------------------------------------
#define LEDpin 2                          // LED测试引脚
int debugflag = 0 ;                       // 串口调试标志
byte TEMPRATURE = 0;                      // 温度
byte HUMIDITY  = 0;                       // 湿度
char col[128] = "A2960110";               // 串口接收数据格式 数据格式 A 29 60 1 1 1 以A开头 温度（0-99） 湿度（0-99）开关1（0-1）开关2（0-1）开关3（0-1）如A2960111-->温度：29 湿度：60 开关1：1 开关2：1 开关3：0
unsigned long lastMs = 0;                 // 时间戳
WiFiClient espClient;                     // WIFI客户端
PubSubClient client(espClient);           // MQTT客户端
static unsigned char TempandHumiLogo[] PROGMEM ={
/*--  调入了一幅图像：C:\Users\zhang\Desktop\boxes\开发图片\demo04.bmp  --*/
/*--  宽度x高度=128x64  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,
0xC6,0x82,0x01,0x1D,0x11,0x82,0xC6,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x010,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x44,0x82,0x82,
0xFE,0x00,0x54,0x92,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0xFE,0x82,0x82,0xBA,0x82,
0xBA,0x82,0xBA,0x82,0xBA,0x82,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xD8,0x04,0x04,0x00,0x04,0x04,0x00,0x00,
0x04,0x04,0xC1,0xA1,0xC5,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x80,0x00,
0x04,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x04,0x04,0x00,0x00,0x04,0x84,0x80,0x80,
0x84,0xC4,0x80,0x80,0x84,0x04,0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x04,0x00,0x00,
0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x00,
0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x00,
0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,
0x00,0x04,0x04,0x00,0x00,0x04,0x98,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xE0,0xD0,0xCF,0x4A,0x4F,0xD0,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x84,0x69,
0x00,0x2F,0x4A,0xFA,0x0A,0xFA,0x4A,0x2F,0x00,0x00,0x00,0x00,0x80,0x7F,0x02,0x2F,
0x6A,0xAA,0xAA,0xAF,0x62,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x73,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x73,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x01,0x03,0xC2,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x80,
0x01,0xC1,0x41,0x41,0x41,0x41,0x41,0xC1,0x01,0x00,0x00,0x00,0x00,0xC0,0x40,0xC0,
0x41,0x60,0x40,0xC0,0x41,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xCE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x68,0x82,0x01,0x40,0x61,0x82,0x68,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC2,0x34,
0x80,0xF7,0x95,0xF5,0x95,0xF5,0x95,0xF7,0x80,0x00,0x00,0x00,0xC0,0x3F,0x01,0x17,
0xB5,0x55,0x55,0x57,0xB1,0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x24,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x39,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x10,0x00,0x00,0x10,0x10,0x00,0x00,
0x10,0x10,0x01,0x01,0x11,0x10,0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,
0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,
0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,
0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,
0x10,0x10,0x00,0x00,0x10,0x00,0x10,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,
0x10,0x10,0x00,0x00,0x10,0x10,0x00,0x00,0x00,0x10,0x10,0x00,0x00,0x10,0x10,0x00,
0x00,0x10,0x10,0x00,0x00,0x10,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x7C,0x66,0x76,0x7F,
0x7E,0x7E,0x7C,0x78,0x70,0x30,0x7C,0x6C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
//----------------------------------------------------------------------------------------------------------------

//响应云端设置函数
void switchSet(String str)

{
   StaticJsonBuffer<100> jsonBuffer;    
   JsonObject& root = jsonBuffer.parseObject(str);
   //----
   if(str.startsWith("{\"Switch1state\"")) //开关1
   {
   int sw1_set = root[String("Switch1state")];
   if(debugflag == 0)Serial.println(sw1_set);
   if(sw1_set == 0)
   {
     Serial.println("10");
     col[5] = '0';
     digitalWrite(LEDpin, HIGH); //拉高电平
     //digitalRead(LEDpin);
   }
   else if(sw1_set == 1)
   {
     Serial.println("11");
     col[5] = '1';
     digitalWrite(LEDpin, LOW); //拉低电平
   } 
   }
   //------
   if(str.startsWith("{\"Switch2state\"")) //开关2
   {
   int sw2_set = root[String("Switch2state")];
   if(debugflag == 0)Serial.println(sw2_set);
   if(sw2_set == 0)
   {
     Serial.println("20");
     col[6] = '0';
     digitalWrite(LEDpin, HIGH); //拉高电平
     //digitalRead(LEDpin);      //读引脚
   }
   else if(sw2_set == 1)
   {
     Serial.println("21");
     col[6] = '1';
     digitalWrite(LEDpin, LOW); //拉低电平
   } 
   }
   //----
    if(str.startsWith("{\"Switch3state\"")) //开关3
   {
   int sw3_set = root[String("Switch3state")];
   if(debugflag == 0)Serial.println(sw3_set);
   if(sw3_set == 0)
   {
     Serial.println("30");
     col[7] = '0';
     digitalWrite(LEDpin, HIGH); //拉高电平
     //digitalRead(LEDpin);
   }
   else if(sw3_set == 1)
   {
     Serial.println("31");
     col[7] = '1';
     digitalWrite(LEDpin, LOW); //拉低电平
   } 
   }
}
/*******************************************************************
 *  void callback(char *topic, byte *payload, unsigned int length)
 *   服务器下发数据解析
 ******************************************************************/
void callback(char *topic, byte *payload, unsigned int length) 
{
 if(debugflag == 0)
 {
   Serial.print("Message arrived [");
   Serial.print(topic);
   Serial.print("] ");
 }
 payload[length] = '\0';
 if(debugflag == 0)Serial.println((char *)payload);
 if (strstr(topic, ALINK_TOPIC_PROP_SET))
 {
    StaticJsonBuffer<100> jsonBuffer;    
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (!root.success()) 
    {
     if(debugflag == 0) Serial.println("parseObject() failed");
     return ;
    } 
   //解析云端下发数据
   String params = root["params"];
   if(debugflag == 0)Serial.println(params);
   else;
   switchSet(params);      //开关数据解析
  }
} 

/*******************************************************************
 *   wifiInit()
 *   建立网络连接
 ******************************************************************/
void wifiInit() 
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  if(debugflag == 0) Serial.println(".");
  }
  if(debugflag == 0)
  {
    Serial.println("Connected to AP"); 
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());/*WiFi初始化*/
  }
  client.setServer(MQTT_SERVER,MQTT_PORT); /*连接MQTT服务器*/ 
  client.setCallback(callback);
}
/*******************************************************************
 *   mqttCheckConnect() 
 *   建立MQTT连接
 ******************************************************************/
void mqttCheckConnect() 
{
  while (!client.connected())
  {
    delay(500);
    if(debugflag == 0) Serial.println("Connecting to MQTT Server ...");
   if (client.connect(CLIENT_ID,MQTT_USRNAME,MQTT_PASSWD))
  {
    if(debugflag == 0) Serial.println("MQTT Connected!");  
    client.subscribe(ALINK_TOPIC_PROP_POSTRSP);
    client.subscribe(ALINK_TOPIC_PROP_SET);
    if(debugflag == 0) Serial.println("subscribe done"); 
   }
    else 
    {
      if(debugflag == 0)
      {
        Serial.print("MQTT Connect err:");
        Serial.println(client.state());
      }
      delay(5000);
    }
  }
}
/*******************************************************************
 *   mqttIntervalPost()
 *   数据推送
 *   params":{"BackPoleState":1,"BackPoleLength":30,"SystemState":1,"WorkState":1,"FrontPoleState":1,"FrontPoleLength":20,"LaserSensorState":0,"StepLadderState":1,"StepLadderLength":40}
 ******************************************************************/
void mqttIntervalPost() { 
  /* char col[128] = "A2960110";打包数据要上传的数据 */
  //{"id":"123","iotId":"pyHC0jntHJ20EAeRrnav000100","method":"thing.event.property.post","params":{"Switch2state":0,"SmokerValue":40,"Switch3state":1,"Humidity":100,"LightValue":30,"Switch1state":0,"Temprature":20}
  char sw1state[32];
  char sw2state[32];
  char sw3state[32];
  char temprature[32];
  char humidity[32];
  char jsonBuf1[256];//packet1
  //char jsonBuf2[256];//packet1
  sprintf(temprature,"{\"Temprature\":%d",((col[1]-48) * 10 + col[2] - 48));
  sprintf(humidity,"\"Humidity\":%d",((col[3] - 48) * 10 + col[4] - 48));
  sprintf(sw1state,"\"Switch1state\":%d",col[5]-48);
  sprintf(sw2state,"\"Switch2state\":%d",col[6]-48);
  sprintf(sw3state, "\"Switch3state\":%d}",col[7]-48);
  sprintf(jsonBuf1, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST,temprature,humidity,sw1state,sw2state,sw3state);
  if(debugflag == 0) Serial.println(jsonBuf1); 
  //if(debugflag == 0) Serial.println(jsonBuf2); 
  client.publish(ALINK_TOPIC_PROP_POST,jsonBuf1);
  //client.publish(ALINK_TOPIC_PROP_POST,jsonBuf2);
}

/*串口接收服务函数*/
void SerialReciver()
{
 // char col[128] = "A2960110";
  int i = 0;
  while(Serial.available()>0) {
  col[i] = Serial.read();      //Serial.read();移除缓存区Serial.peek()不会
  delay(2);
  i++;
  }
 }
 /***界面绘图***/
 void oledInitMenu()
 {
     oled.clearDisplay();          // Clear screen
     oled.drawBitmap(TempandHumiLogo, 1024);
     oled.setTextXY(2,11);         // Set cursor position
     oled.putString(" *C"); 
     oled.setTextXY(4,11 );        // Set cursor position 
     oled.putString(" %R");  
 }
 void oledRefrashData()
 {
      TEMPRATURE = (col[1]-48) * 10 + col[2] - 48;
      HUMIDITY = (col[3] - 48) * 10 + col[4] - 48;
      oled.setTextXY(2,9);            // Set cursor position 
      oled.putString("  ");           // Clear that area
      oled.setTextXY(2,9); 
      oled.putNumber(int(TEMPRATURE));
      oled.setTextXY(4,9); 
      oled.putString("  "); 
      oled.setTextXY(4,9); 
      oled.putNumber(int(HUMIDITY)); // OLED显示温湿度
 }
/****************************************************
 *    main() 函数
 *    init and whlie
*****************************************************/
void setup() 
{
  /*想看调试信息请debugflag == 0*/
  Serial.begin(112500);        //波特率
  if(debugflag == 0) Serial.println("Start");
  pinMode(LEDpin, OUTPUT);      // Initialize the LEDpin
  digitalWrite(LEDpin, HIGH);    //拉低电平
  Wire.begin();                 //打开iic通讯，为读取内部地址
  oled.init();                  //屏幕初始化
  oledInitMenu();               //界面绘图
  wifiInit();
} 
void loop() 
{
  SerialReciver();                 //串口服务
  if (millis() - lastMs >= 20000)  //20秒轮循
  {
    if(debugflag == 0) Serial.print("recieved:");
    if(debugflag == 0) Serial.println(col);
    lastMs = millis();
    mqttCheckConnect(); 
    mqttIntervalPost(); 
    oledRefrashData(); 
  }
  client.loop();    //MQTT客户端循环*/  
}
