// Use if you want to force the software SPI subsystem to be used for some reason (generally, you don't)
// #define FASTLED_FORCE_SOFTWARE_SPI
// Use if you want to force non-accelerated pin access (hint: you really don't, it breaks lots of things)
// #define FASTLED_FORCE_SOFTWARE_SPI
// #define FASTLED_FORCE_SOFTWARE_PINS
#include <FastLED.h>
#define LED_PIN     3
#define NUM_LEDS    60
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS  255   //MAX 255
CRGB leds[NUM_LEDS];

int myLEDs [60][3] = {};   //initiate an array with 60 leds with 3 values for color
bool debug = false;
float bMin = 0.9;          //this used to be work as the leds min brightness but because of certain 
                           //things its now just another parameter to adjust the LEDs temp brightness. Try to fix it I dare you
int nodeLen = 32;
int delayTime = 75;

//float m = (1 - (((1 - bMin) / 2) + bMin)) / atan(nodeLen / 2);    //Used for tan
float m = ((nodeLen / 2) / (1 -( ((1-bMin)/2) + bMin)));

/*
move top to bottom
work one colomn at a time
three types of changes
  combine colors
  shift colors down
  generate new colors

those are gonna have to be three functions
these actions will be the same for an led every time

every fourth LED will be the combination of two others
  except for the last 8 colums


 LED visual arrangment
  H I J K L M N O
   D   E   F   G
     B       C
         A

LED combinations
  letter   pos number
  A = B C  3  = 7  11
  
  B = D E  7  = 15 19
  C = F G  11 = 23 27
  
  D = H I  15 = 31 35
  E = J K  19 = 39 43
  F = L M  23 = 47 51
  G = N O  27 = 55 59

  A will be the combination of B and C
  I didnt want to use '+' since it would be confusing in the "pos number" diagram

  LED = (LED*2)+1 (LED*2)+1+4



  A = B C  3  = 8  4
  
  B = D E  7  = 16 12
  C = F G  11 = 24 20
  
  D = H I  15 = 32 28
  E = J K  19 = 40 36
  F = L M  23 = 48 44
  G = N O  27 = 56 52

  LED = (LED*2)+2 (LED*2)+2-4


LED arragnment in column
  A3
  A2
  A1
  A0

LED arrangment in array
  ONMLKJIH GFED CB A
    spaces are ment to represent different rows

*/



void setup() {
    randomSeed(analogRead(0));
    Serial.begin(9600);
    FastLED.setBrightness(BRIGHTNESS);
    LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    fill_solid(leds, NUM_LEDS, CRGB(0,0,0));

    //set every value in the array to 0
    for(int i = 0; i < 60; i++){
      for(int j = 0; j < 3; j++){
        myLEDs[i][j] = 0;
      }
    }
}

void debugLED(int led, String funcName){
  if(debug == true){
    Serial.print("led: ");
    Serial.print(led);
    Serial.print("  color: ");
    Serial.print(leds[led][0]);
    Serial.print(", ");
    Serial.print(leds[led][1]);
    Serial.print(", ");
    Serial.print(leds[led][2]);
    Serial.print("          ");
    Serial.println(funcName);
  }
}

int generateColor(int led){
  debugLED(led, "generate");
  
  for(int i = 0; i < 3; i++){
    myLEDs[led][i] = random(10,45);
  }
  myLEDs[led][random(0,3)] = random(50,75);
}



int generateFadeColor(int led, int dist){ //dist = how far the LED is from the orignal color its fading from
  debugLED(led, "generateFade");
  
  float bTemp = ( (-1 * (((dist) - (nodeLen / 2)) / m)) + (((1 - bMin) / 2) + bMin) );
//  Serial.print("led % nodeLen: ");
//  Serial.println(led % nodeLen);
//  Serial.print("bTemp: ");
//  Serial.println(bTemp);
  for(int i = 0; i < 3; i++){
//    Serial.println(myLEDs[led][i]);
//    myLEDs[led][i] = myLEDs[led - dist][i] * bTemp;
    myLEDs[led][i] = myLEDs[led][i] * bTemp;
//    Serial.println(myLEDs[led][i]);
  }
//  Serial.println();
}

int generateBlankColor(int led){

  debugLED(led, "generateBlank");
  
  for(int i = 0; i < 3; i++){
    myLEDs[led][i] = 0;
  }
}


int shiftColor(int led){

  debugLED(led, "shift");
  
  for(int i = 0; i < 3; i++){
    myLEDs[led][i] = myLEDs[led+1][i];
  }
}


int combineColor(int led){

  debugLED(led, "combine");

//  LED = (LED*2)-1 (LED*2)-1+4
  for(int i = 0; i < 3; i++){
    int setColor = myLEDs[(led * 2)+ 2][i] + myLEDs[(led * 2)- 2][i];
    if(setColor > 255){
      setColor = 255;
    }
    myLEDs[led][i] = setColor;
  }
}


int runs = 0;
void loop() {
  int currentLED = 0;

  for(int i = 0; i < 7; i++){
  
    for(int i = 0; i < 3; i++){
      shiftColor(currentLED);
      currentLED++;
    }
    combineColor(currentLED);
    currentLED++;
  }
  for(int i = 0; i < 8; i++){
    for(int i = 0; i < 3; i++){
      shiftColor(currentLED);
      currentLED++;
    }
    if(((runs % nodeLen) == 0) || runs == 0){
      generateColor(currentLED);
    }else{
      generateFadeColor(currentLED, runs % nodeLen);
    }
  currentLED++;
  }

  //So this is some bad coding but leds[] is used by the library to define the actual leds in the strip
  //myLEDs is the array I made to contain all the values for the leds
  
  for(int i = 0; i < 60; i++){
    int r = myLEDs[i][0];
    int g = myLEDs[i][1];
    int b = myLEDs[i][2];

    leds[i] = CRGB(r, g, b);
  }
  
  FastLED.show();

//  Serial.println();
//  Serial.print("runs: ");
//  Serial.println(runs);
//  Serial.println();
//  Serial.println();

  delay(delayTime);
  runs++;
}
