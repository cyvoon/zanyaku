//#include <Arduino.h>
#include <WioLTEforArduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/dtostrf.h>
#include <ADXL345.h>  

#define INTERVAL    (5000)


void AE_HX711_Init(void);
void AE_HX711_Reset(void);
long AE_HX711_Read(void);
long AE_HX711_Averaging(long adc,char num);
float AE_HX711_getGram(char num);

//---------------------------------------------------//
// ピンの設定
//---------------------------------------------------//
#define pin_data  (WIOLTE_D20)
#define pin_clk   (WIOLTE_D19)

//---------------------------------------------------//
// ロードセル　Ｓ字型　ＳＣ３０１Ａ　１００ｋＧ [P-12036]
//---------------------------------------------------//
//#define OUT_VOL   0.002f      //定格出力 [V]
//#define LOAD      100000.0f   //定格容量 [g]

//---------------------------------------------------//
// ロードセル　シングルポイント（ ビーム型）　ＳＣ６０１　１２０ｋＧ [P-12035]
//---------------------------------------------------//
//#define OUT_VOL   0.001f      //定格出力 [V]
//#define LOAD      120000.0f   //定格容量 [g]

//---------------------------------------------------//
// ロードセル　シングルポイント（ ビーム型）　ＳＣ１３３　２０ｋＧ [P-12034]
//---------------------------------------------------//
//#define OUT_VOL   0.002f      //定格出力 [V]
//#define LOAD      20000.0f    //定格容量 [g]

//---------------------------------------------------//
// ロードセル　シングルポイント（ビーム型）　ＳＣ６１６Ｃ　５００ｇ[P-12532]
//---------------------------------------------------//
//#define OUT_VOL   0.00095f   //定格出力 [V]
//#define LOAD      500.0f    //定格容量 [g]

//---------------------------------------------------//
// ロードセル　シングルポイント　 RB-Phi-203　100ｇ
//---------------------------------------------------//
#define OUT_VOL   0.0006f   //定格出力 [V]
#define LOAD      100.0f    //定格容量 [g]

float offset;
WioLTE Wio;
ADXL345 Accel;

void setup() {
  delay(200);
  
  SerialUSB.begin(9600);
  SerialUSB.println("AE_HX711 test");

  SerialUSB.println("--- START ---------------------------------------------------");
  SerialUSB.println("### I/O Initialize.");  
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(500);
  Accel.powerOn();

  SerialUSB.println("### Setup completed.");
  
  SerialUSB.println("### Turn on or reset.");
 if (!Wio.TurnOnOrReset()) {
   SerialUSB.println("### ERROR! ###");
   return;
 }

 SerialUSB.println("### Connecting to \"soracom.io\".");
 delay(5000);
 if (!Wio.Activate("soracom.io", "sora", "sora")) {
   SerialUSB.println("### ERROR! ###");
   return;
 } 


  AE_HX711_Init();
  AE_HX711_Reset();
  offset = AE_HX711_getGram(30); 
}

void loop() {
  char axisdata[100];  
  char weightdata[100];  
 //-------------------------------------------
 //--- Open the connection for soracom.io ----
 //-------------------------------------------

SerialUSB.println("### Open.");
  int connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR : Fail socket open ###");
    goto err;
 }

 

 //-------------------------------------------
 //--- Send the accelerometer data for soracom.io 
 //-------------------------------------------
  int x;
  int y;
  int z;
  Accel.readXYZ(&x, &y, &z);
  SerialUSB.print(x);
  SerialUSB.print(' ');
  SerialUSB.print(y);
  SerialUSB.print(' ');
  SerialUSB.println(z);
  

 
 SerialUSB.println("### Send.");
 sprintf(axisdata, "{\"XYZdata\":%ld}", x);
 SerialUSB.print("Send:");
 SerialUSB.print(axisdata);
 SerialUSB.println("");
 if (!Wio.SocketSend(connectId, axisdata)) {
   SerialUSB.println("### ERROR! ###");
   goto err;
 }
 
 //--- getting weight data from Load-cell ----
 
  float data;
  char S1[20];
  char s[20];
  data = AE_HX711_getGram(5);
  //sprintf(S1,"%s [g] (0x%4x)",dtostrf((data-offset), 5, 3, s),AE_HX711_Read());
  SerialUSB.println(S1);

  sprintf(S1,"%s",dtostrf((data-offset), 5, 3, s));
//-------------------------------------------
 //--- Send the weight data for soracom.io ---
 //-------------------------------------------

 SerialUSB.println("### Send weight data.");
 sprintf(weightdata, "{\"Weight data\":%s}", S1);
 SerialUSB.print("Send:");
 SerialUSB.print(weightdata);
 SerialUSB.println("");
 if (!Wio.SocketSend(connectId, weightdata)) {
   SerialUSB.println("### ERROR! ###");
   goto err;
 }

 
  err:
  delay(INTERVAL);
}



void AE_HX711_Init(void)
{
  pinMode(pin_clk, OUTPUT);
  pinMode(pin_data, INPUT);
}

void AE_HX711_Reset(void)
{
  digitalWrite(pin_clk,1);
  delayMicroseconds(100);
  digitalWrite(pin_clk,0);
  delayMicroseconds(100); 
}

long AE_HX711_Read(void)
{
  long data=0;
  while(digitalRead(pin_data)!=0);
  delayMicroseconds(10);
  for(int i=0;i<24;i++)
  {
    digitalWrite(pin_clk,1);
    delayMicroseconds(5);
    digitalWrite(pin_clk,0);
    delayMicroseconds(5);
    data = (data<<1)|(digitalRead(pin_data));
  }
  //SerialUSB.println(data,HEX);   
  digitalWrite(pin_clk,1);
  delayMicroseconds(10);
  digitalWrite(pin_clk,0);
  delayMicroseconds(10);
  return data^0x800000; 
}


long AE_HX711_Averaging(long adc,char num)
{
  long sum = 0;
  for (int i = 0; i < num; i++) sum += AE_HX711_Read();
  return sum / num;
}

float AE_HX711_getGram(char num)
{
  #define HX711_R1  20000.0f
  #define HX711_R2  8200.0f
  #define HX711_VBG 1.25f
  #define HX711_AVDD      4.2987f//(HX711_VBG*((HX711_R1+HX711_R2)/HX711_R2))
  #define HX711_ADC1bit   HX711_AVDD/16777216 //16777216=(2^24)
  #define HX711_PGA 128
  #define HX711_SCALE     (OUT_VOL * HX711_AVDD / LOAD *HX711_PGA)
  
  float data;

  data = AE_HX711_Averaging(AE_HX711_Read(),num)*HX711_ADC1bit; 
  //SerialUSB.println( HX711_AVDD);   
  //SerialUSB.println( HX711_ADC1bit);   
  //SerialUSB.println( HX711_SCALE);   
  //SerialUSB.println( data);   
  data =  data / HX711_SCALE;


  return data;
}
