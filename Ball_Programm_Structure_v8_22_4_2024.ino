/**/
//Filename: Ball_Programm_Arduino
//Author: Luis Ritter
//Created On: 30. October 2023 16:08
//Description: Ball Programm. Comunication and execute LED and Tone
/**/

//Includes 
 //BLE Includes 
  #include <ArduinoBLE.h> //under ArduinoBLE 
  #include <math.h>       //already included
  #include <Arduino.h>    //already Included

 //Accelerator Include
  #include <Arduino_LSM9DS1.h>  //under Arduino_LSM9DS1

 //LED Include
  #include <Adafruit_NeoPixel.h> //under adafruit neopixel (by Adafruit)

 //Tone Include
  #if !( ARDUINO_ARCH_NRF52840 && TARGET_NAME == ARDUINO_NANO33BLE )
  #error This code is designed to run on nRF52-based Nano-33-BLE boards using mbed-RTOS platform! Please check your Tools->Board setting.
  #endif

  #define _PWM_LOGLEVEL_       1
  // To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
  #include "nRF52_MBED_PWM.h"     //under nRF52_MBED_PWM






// Defines
  enum state {Bluetooth, Sensor, Ton, LED, Akku, OffMode};
  state state = Bluetooth;

 //BLE Defines
  
  #define SERVICE_UUID         "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
  #define CHARACTERISTIC_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8" 
  #define CHARACTERISTIC2_UUID "4a78b8dd-a43d-46cf-9270-f6b750a717c8" 

  BLEService pService(SERVICE_UUID); 
  BLEUnsignedIntCharacteristic xCharacteristic(CHARACTERISTIC_UUID, BLEBroadcast | BLERead | BLEWrite); 
  BLEUnsignedIntCharacteristic yCharacteristic(CHARACTERISTIC2_UUID, BLEBroadcast | BLERead | BLEWrite); 

 //LED Defines
  #define LEDPIN  4
  #define NUMLEDs 16

  Adafruit_NeoPixel LEDs(NUMLEDs, LEDPIN, NEO_GRB + NEO_KHZ800);

 //Tone Defines 
  uint32_t Tone_Pin  = D3;
  float StepsFreq = 40; 

 //Akku Defines
  #define AkkuRead_Pin A1



// Variablen
 //BLE Variablen

 

  //BLE eingabe werte 
    unsigned long xCharact = 0;
    uint8_t aByteZ = 0;
    byte aByteAr[3] = {0,0,0};
    byte bByte = 0;
    byte cByte = 0;
    byte dByte = 0;
    unsigned int pruefziffer = 0;

 //Berechungs Bariablen 
  
  byte Volume = 0;
  byte FreqStill = 0;
  byte FreqMov = 0;
  bool Beep = 0;         // on/off
  byte BeepStill = 0;
  byte BeepMov = 0;
  byte LEDRedStill = 0;
  byte LEDGreenStill = 255;
  byte LEDBlueStill = 0;
  byte LEDRedMov = 0;
  byte LEDGreenMov = 0;
  byte LEDBlueMov = 255;
  byte Brightness = 100;
  bool LEDFlashing = 0;
  byte LEDFlashFreq = 0;

 //Sensor Variablen 
  float accx, accy, accz, accsum;
  float altaccsum; 
  unsigned long AccTime = 0; 

  
 //Ton Variablen 
  float dutyCycle = 50.0f;
  float freq      = 800.0f;
  mbed::PwmOut* pwm   = NULL;
  bool ToneToggle = 0;
  unsigned long ToneTime = 0;
  int BeepSound;
  float OldFreq; 
  float OlddutyCycle;
  

 //LED Variablen 
  int LEDRed, LEDGreen, LEDBlue;
  unsigned long LEDFlashTime; 
  bool LEDFlashOF;

 //Akku Variablen

  int Akkuvalue = 0;  
  byte Akkuprocent = 0; 
  byte bAkkuByte = 0; 
  unsigned int AkkuPruefziffer = 0;
  byte aAkkuByte = 0;
  unsigned long AkkuValue = 0;
  bool LowBatterieToggle = 1; 
  int LowBatterieSwitch = 0;
  unsigned long LowBatterieTime = 0;
  int AkkuZw = 0;


void setup() {
 // put your setup code here, to run once:

 // Bluetooth Setup 

  if (!BLE.begin()) {

    while (1);
  }


  BLE.setLocalName("Klingelball");
  BLE.setDeviceName("Klingelball");
  
  BLE.setAdvertisedService(pService);

  pService.addCharacteristic(xCharacteristic);  //muss hier sein sonst funktioniert es nicht
  pService.addCharacteristic(yCharacteristic);

  BLE.addService(pService);
  BLE.setAppearance(0x80);

  xCharacteristic.setValue(0);
  yCharacteristic.setValue(0);

  BLE.advertise();
 
 //Accelerator 
  
  if (!IMU.begin()) {
    
    while (1);
  }


  // Pin Setup 
   //LED Pins
   
   //Ton Pins
    
    setPWM(pwm, Tone_Pin, freq, dutyCycle);
    pinMode(Tone_Pin, OUTPUT);

   //Akku Pins
    pinMode(AkkuRead_Pin, INPUT);

   


}

void loop() {
 // put your main code here, to run repeatedly:

  switch(state){

    case Bluetooth: //Bluetooth loop 

     //BLE Programm 
      BLE.stopAdvertise();//damit es immer neustartet es beendet sich selbst sonst
      BLE.advertise();
      if(BLE.connected()){

       //eingabe werte von BLE in Variablen speichern 
        xCharact = xCharacteristic.value();
      
       //die einzelnen bytes in array und Varbiablen speichern
        aByteZ = xCharact;
        for(int i = 0; i <= 2; i++){  
          aByteAr[i] = aByteZ % 10; 
          aByteZ = aByteZ / 10;
        }
        bByte = (xCharact >> 8);  
        cByte = (xCharact >> 16);
        dByte = (xCharact >> 24);


       //Prüfziffer berechnen 
        pruefziffer = 0;
        pruefziffer += bByte * 1;
        pruefziffer += cByte * 3;
        pruefziffer += dByte * 1;
        

        pruefziffer = 9 - (pruefziffer % 10); 

       //überprüfung 
        if(xCharact > 3){

          if(aByteAr[0] == pruefziffer){
            
          //Switch Welche eingabe 

            switch((aByteAr[2]*10 + aByteAr[1])){
              default:

              break; 
              case 0: 

                if(bByte == 0) state = OffMode; 
                else           state = Bluetooth;
                

              break; 
              case 1:

                Volume = bByte;
                FreqStill = cByte;
                FreqMov = dByte;

              break;
              case 2:
                
                Beep = bByte;         // on/off
                BeepStill = cByte;
                BeepMov = dByte;

              break;
              case 3:

                LEDRedStill = bByte;
                LEDGreenStill = cByte;
                LEDBlueStill = dByte;

              break;
              case 4:

                LEDRedMov = bByte;
                LEDGreenMov = cByte;
                LEDBlueMov = dByte;

              break;
              case 5:

                Brightness = bByte;
                LEDFlashing = cByte;
                LEDFlashFreq = dByte;

              break;
            }

            xCharacteristic.setValue(1);

          }else{
            xCharacteristic.setValue(2);

          }
        }
        
      }


      if(state < 5){
        state = Sensor;
      }
    break; 


    case Sensor: // Sensoren auslesen 

      if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(accx, accy, accz);
      }

      if(accx < 0){ accx = accx * -1; } 
      if(accy < 0){ accy = accy * -1; }
      if(accz < 0){ accz = accz * -1; }

      accsum = (accx + accy + accz) / 3; 
      
      if(accsum > altaccsum){
        AccTime = millis(); 
        altaccsum = accsum; 
      }

      if(AccTime + 2000 < millis()){
        altaccsum = 0; 
      }

      accsum = altaccsum; 


      state = Ton;
    break; 



    case Ton: // Anpassung Ton 
      if(millis() >= LowBatterieTime + 1000){
        dutyCycle = (Volume/10); // interpolieren 

        freq = map(accsum, 0, 4, FreqStill*70, FreqMov*70) +31;

        BeepSound = map(accsum, 0, 4, BeepStill, BeepMov);

        if(dutyCycle >= 1){
          if(BeepSound <= 1){
            if(OldFreq > (freq + StepsFreq) || OldFreq < (freq - StepsFreq)){
              OldFreq = freq; 
              OlddutyCycle = dutyCycle;
              setPWM(pwm, Tone_Pin, freq, dutyCycle);//starts tone   
            }

          }else{
            if(millis() > (ToneTime + BeepSound*25)){
              ToneTime = millis();
              ToneToggle = !ToneToggle; 
              OlddutyCycle = dutyCycle;   
            }

              switch(ToneToggle){
                case 0: 
                  if(OldFreq > (freq + StepsFreq) || OldFreq < (freq - StepsFreq)){
                    OldFreq = freq; 
                    setPWM(pwm, Tone_Pin, freq, dutyCycle);//starts tone
                    OlddutyCycle = dutyCycle;
                  }
                break;
                case 1: 
                  stopPWM(pwm, Tone_Pin);  //stops tone
                  OldFreq = 10000; 
                  OlddutyCycle = dutyCycle;
                break; 
            }
          }
          if(OlddutyCycle +1 < dutyCycle || OlddutyCycle -1 > dutyCycle){
            setPWM(pwm, Tone_Pin, freq, dutyCycle);
            OlddutyCycle = dutyCycle;
          }

        }else{
          stopPWM(pwm, Tone_Pin);  //stops tone
        }
      }
      

      state = LED;
    break;

    case LED: // Anpassung LEDs 
      if(millis() >= LowBatterieTime + 1000){

        LEDRed = map(accsum, 0, 4, LEDRedStill, LEDRedMov);
        LEDGreen = map(accsum, 0, 4, LEDGreenStill, LEDGreenMov);
        LEDBlue = map(accsum, 0, 4, LEDBlueStill, LEDBlueMov);

        LEDs.setBrightness(Brightness);


        if(LEDFlashing == 1){
          if(millis() >= LEDFlashTime + LEDFlashFreq * 1){
            LEDFlashOF = !LEDFlashOF;
            if(LEDFlashOF){
              LEDs.fill(LEDs.Color(LEDRed, LEDGreen, LEDBlue), 0, NUMLEDs);
              LEDs.show();
            }else{
              LEDs.clear();
              LEDs.show();
            }
          }

        }else {
          LEDs.fill(LEDs.Color(LEDRed, LEDGreen, LEDBlue), 0, NUMLEDs);
          LEDs.show();
        }
      }

      state = Akku;
    break; 

    case Akku:// Akkustand (LEDs und Ton aus)

      Akkuvalue = analogRead(AkkuRead_Pin);
      if(Akkuvalue > 738){
        bAkkuByte = ((Akkuvalue-738) / 1.2) +3.6;
      }else{
        bAkkuByte = (Akkuvalue-620) / 30;
      }
      
      AkkuPruefziffer = bAkkuByte * 1;
      AkkuPruefziffer = 9 - (AkkuPruefziffer % 10); 
      aAkkuByte = 60 + AkkuPruefziffer; 

      AkkuValue = 0x0;
      AkkuValue = aAkkuByte + (bAkkuByte << 8); 

      yCharacteristic.setValue(AkkuValue);

      if(bAkkuByte < 20 && LowBatterieToggle){

        if(millis() >= LowBatterieTime + 750){
          LowBatterieTime = millis();

          switch(LowBatterieSwitch){
            case 0: 
              setPWM(pwm, Tone_Pin, 2000, 50);
              LEDs.fill(LEDs.Color(255, 0, 0), 0, NUMLEDs);
              LEDs.show();
              LowBatterieSwitch = 1; 
            break; 
            case 1:
              stopPWM(pwm, Tone_Pin);  //stops tone
              LEDs.clear();
              LEDs.show();
              LowBatterieSwitch = 2; 
            break;
            case 2: 
              setPWM(pwm, Tone_Pin, 2000, 50);
              LEDs.fill(LEDs.Color(255, 0, 0), 0, NUMLEDs);
              LEDs.show();
              LowBatterieSwitch = 3; 
            break; 
            case 3:
              stopPWM(pwm, Tone_Pin);  //stops tone
              LEDs.clear();
              LEDs.show();
              LowBatterieSwitch = 4; 
            break;
            case 4: 
              setPWM(pwm, Tone_Pin, 2000, 50);
              LEDs.fill(LEDs.Color(255, 0, 0), 0, NUMLEDs);
              LEDs.show();
              LowBatterieSwitch = 5; 
            break; 
            case 5:
              stopPWM(pwm, Tone_Pin);  //stops tone
              LEDs.clear();
              LEDs.show();
              LowBatterieSwitch = 6; 
            break;
            case 6: 
              setPWM(pwm, Tone_Pin, 2000, 50);
              LEDs.fill(LEDs.Color(255, 0, 0), 0, NUMLEDs);
              LEDs.show();
              LowBatterieSwitch = 7; 
            break; 
            case 7:
              stopPWM(pwm, Tone_Pin);  //stops tone
              LEDs.clear();
              LEDs.show();
              LowBatterieSwitch = 8; 
            break;
            case 8: 
              setPWM(pwm, Tone_Pin, 2000, 50);
              LEDs.fill(LEDs.Color(255, 0, 0), 0, NUMLEDs);
              LEDs.show();
              LowBatterieSwitch = 9; 
            break; 
            case 9:
              stopPWM(pwm, Tone_Pin);  //stops tone
              LEDs.clear();
              LEDs.show();
              LowBatterieSwitch = 10; 
            break;
            case 10: 
              setPWM(pwm, Tone_Pin, 2000, 50);
              LEDs.fill(LEDs.Color(255, 0, 0), 0, NUMLEDs);
              LEDs.show();
              LowBatterieSwitch = 11; 
            break; 
            case 11:
              stopPWM(pwm, Tone_Pin);  //stops tone
              LEDs.clear();
              LEDs.show();
              LowBatterieSwitch = 0; 
              LowBatterieToggle = 0;
            break;
          }
        }

      }else if(bAkkuByte > 50){
        LowBatterieToggle = 1;
        LowBatterieSwitch = 0; 
      }

     
      state = Bluetooth;
    break;



    case OffMode: // Energiesparmodus (Wenn ball nicht in betrieb Per handy einstellbar)
      // entweder das oder das andere (außer bluetooth);  entweder nur bluetooth an oder nur sensoren 

      LEDs.clear();
      LEDs.setBrightness(0);
      LEDs.show();
      OldFreq = 10000;

      stopPWM(pwm, Tone_Pin);


      //BLE Programm 
      BLE.stopAdvertise();//damit es immer neustartet es beendet sich selbst sonst
      BLE.advertise();
      if(BLE.connected()){

        //eingabe werte von BLE in Variablen speichern 
          xCharact = xCharacteristic.value();
        
        //die einzelnen bytes in array und Varbiablen speichern
        
          aByteZ = xCharact & 0xFF;
          for(int i = 0; i <= 2; i++){  
            aByteAr[i] = aByteZ % 10; 
            aByteZ = aByteZ / 10;
          }
          bByte = (xCharact >> 8);  
          cByte = (xCharact >> 16);
          dByte = (xCharact >> 24);


        //Prüfziffer berechnen 
          pruefziffer = 0;
          pruefziffer += bByte * 1;
          pruefziffer += cByte * 3;
          pruefziffer += dByte * 1;
          
          pruefziffer = 9 - (pruefziffer % 10); 


          if(xCharact > 3){

          //überprüfung 
            if(aByteAr[0] == pruefziffer){
              
            //Switch Welche eingabe 

              switch((aByteAr[2]*10 + aByteAr[1])){
                default:

                break; 
                case 0: 

                  if(bByte == 0){
                    state = OffMode; 
                  }else{
                    state = Sensor;
                  }

                break; 
                case 1:

                  Volume = bByte;
                  FreqStill = cByte;
                  FreqMov = dByte;

                break;
                case 2:
                  
                  Beep = bByte;         // on/off
                  BeepStill = cByte;
                  BeepMov = dByte;

                break;
                case 3:

                  LEDRedStill = bByte;
                  LEDGreenStill = cByte;
                  LEDBlueStill = dByte;

                break;
                case 4:

                  LEDRedMov = bByte;
                  LEDGreenMov = cByte;
                  LEDBlueMov = dByte;

                break;
                case 5:

                  Brightness = bByte;
                  LEDFlashing = cByte;
                  LEDFlashFreq = dByte;

                break;
              }
              xCharacteristic.setValue(1);

            }else{              
              xCharacteristic.setValue(2);

            }
          }

        Akkuvalue = analogRead(AkkuRead_Pin);
      if(Akkuvalue > 738){
        bAkkuByte = ((Akkuvalue-738) / 1.2) +3.6;
      }else{
        bAkkuByte = (Akkuvalue-620) / 30;
      }

        AkkuPruefziffer = bAkkuByte * 1;
        AkkuPruefziffer = 9 - (AkkuPruefziffer % 10); 
        aAkkuByte = 60 + AkkuPruefziffer; 

        AkkuValue = 0x0;
        AkkuValue = aAkkuByte + (bAkkuByte << 8); 

        yCharacteristic.setValue(AkkuValue);

      }

    break; 


  }

  if(millis() < 100) {
    AccTime = 0; 
    ToneTime = 0;
    LEDFlashTime = 0;
    LowBatterieTime = 0;
  }

}

