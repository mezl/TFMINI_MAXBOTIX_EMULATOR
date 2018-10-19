#include <Wire.h>
#define I2C_SLAVE_ADDR  0x70  //Standard MaxsonarI2CXL address 0x70 

#define MICROVIEW
#ifdef MICROVIEW
  #include <MicroView.h>
  MicroViewWidget *widget,*widget2,*widget3;
#endif

uint8_t LED = 13;//for pro mini
byte received;
uint16_t distance;      // Distance value in 16 bit word
uint8_t Scaling = 1 ;  //Benewake outputs in cm, we don't have to scale
uint16_t strength;
uint16_t strLimit = 20;
uint16_t rangeMax = 1200;
//range 30cm ~ 1200cm for indoor 700cm for outdoor
/* 
This is the structure of the Benewake TFMINI serial message
Byte1-2   Byte3   Byte4   Byte5       Byte6        Byte7      Byte8     Byte9
0x59 59   Dist_L  Dist_H  Strength_L  Strength_H   Reserved   Raw.Qual  CheckSum_
*/

void readlaser(){     
    while(Serial.available()>=9)
    {
        if((0x59 == Serial.read()) && (0x59 == Serial.read())) //Byte1 & Byte2, 0x59 = 'Y'
        {
            unsigned int b3 = Serial.read(); //Byte3 Dist_L distance value is a low 8-bit.
            unsigned int b4 = Serial.read(); //Byte4 Dist_H distance value is a high 8-bit.
            unsigned int b5 = Serial.read(); //Byte5 Strength_L is a low 8-bit.
            unsigned int b6 = Serial.read(); //Byte6 Strength_H is a high 8-bit.
            unsigned int b7 = Serial.read(); //Byte7 Integration time.
            unsigned int b8 = Serial.read(); //Byte8 Reserved bytes.
            unsigned int b9 = Serial.read(); //Byte9 Checksum parity.
            unsigned int checkSum = 0x59 + 0x59 + b3 + b4 + b5 + b6 + b7 + b8;
            if(b9 == (checkSum & 0xff)){
              digitalWrite(LED, !digitalRead(LED));  //toggle LED
              distance = (b3+b4*256)/Scaling;
              strength = b5+b6*256;

              if ( (strength < strLimit))  {
                    distance = rangeMax;
              }
                          
              
            }
        }
    }
}

/*
 The default I2C-Address of the sensor is 0x70. 
 To perform a range measurement you must send the "Take Range Reading” command byte 0x5
 */

void receiveEvent(int howMany) {
  while (Wire.available()) { // loop through all but the last
        {         
      received = Wire.read(); 
      if (received == 0x51)
      {
      //Serial.println(received);    
      }
    }
  }
}

void requestEvent() 
{
Wire.write (highByte(distance));
Wire.write (lowByte(distance));
}


void setup() {
 pinMode (LED, OUTPUT);
 digitalWrite(LED, LOW);
 
 Serial.begin(115200);
 Wire.begin(I2C_SLAVE_ADDR);
 Wire.onReceive(receiveEvent); // register event
 Wire.onRequest(requestEvent);

 #ifdef MICROVIEW
  uView.begin();              // start MicroView
  uView.clear(PAGE);          // clear page
  uView.setFontType(0);
  uView.print("Start Reading");   // display string
  uView.display();
 #endif
 delay(1000);  
 uView.clear(PAGE);          // clear page
}
/*
MicroView FontType:
0:font5x7
1:font8x16
2:sevensegment
3:fontlargenumber
4:space01
5:space02
6:space03
7:

*/
int counter = 0;
char loading[5]={'-','\\','|','/',' '};

#ifdef MICROVIEW
int w = uView.getLCDWidth();
int h = uView.getLCDHeight();
#endif

void loop() {

  readlaser();
  #ifdef MICROVIEW
   char buffer[6];   
   sprintf(buffer, "%3d", distance);
   if(counter++ > 40){counter = 0;}   
   uView.clear(PAGE);          // clear page
   uView.setFontType(3);//fontlargenumber
   uView.setCursor(0,0);
   uView.print(buffer);//draw large number of distance
   
   uView.setFontType(0);//font5x7   
   
   sprintf(buffer, "%5d", strength);
   uView.setCursor(w-uView.getFontWidth()*6,0);//Top Right   
   uView.print(buffer);//draw small numver for strength
   uView.setCursor(60,40);     
   int cnt = counter/10;
   uView.print(loading[cnt]);//do loading animation
   uView.display(); 
  #endif
}

