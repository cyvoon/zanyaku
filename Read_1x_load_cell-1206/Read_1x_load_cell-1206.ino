//-------------------------------------------------------------------------------------
// HX711_ADC.h
// Arduino master library for HX711 24-Bit Analog-to-Digital Converter for Weigh Scales
// Olav Kallhovd sept2017
// Tested with      : HX711 asian module on channel A and YZC-133 3kg load cell
// Tested with MCU  : Wio LTE
//-------------------------------------------------------------------------------------
// This is an example sketch on how to use this library
// Settling time (number of samples) and data filtering can be adjusted in the HX711_ADC.h file
#include <WioLTEforArduino.h>
#include <HX711_ADC.h>

//HX711 constructor (dout pin, sck pin)
//---------------------------------------------------//
// ピンの設定
//---------------------------------------------------//
#define pin_data  (WIOLTE_D20)
#define pin_clk   (WIOLTE_D19)

HX711_ADC LoadCell(pin_data, pin_clk);

long t;

void setup() {
  SerialUSB.begin(9600);
  SerialUSB.println("Wait...");
  LoadCell.begin();
  long stabilisingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilising time
  LoadCell.start(stabilisingtime);
  LoadCell.setCalFactor(696.0); // user set calibration factor (float)
  SerialUSB.println("Startup + tare is complete");
}

void loop() {
  //update() should be called at least as often as HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS
  //longer delay in scetch will reduce effective sample rate (be carefull with delay() in loop)
  LoadCell.update();

  //get smoothed value from data set + current calibration factor
  if (millis() > t + 250) {
    float i = LoadCell.getData();
    SerialUSB.print("Load_cell output val: ");
    SerialUSB.println(i);
    t = millis();
  }

  //receive from SerialUSB terminal
  if (SerialUSB.available() > 0) {
    float i;
    char inByte = SerialUSB.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  //check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    SerialUSB.println("Tare complete");
  }

}
