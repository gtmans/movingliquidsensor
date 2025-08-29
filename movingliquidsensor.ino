/* 
moving liquidsensor gtmans 2025
parts:
- 28BYJ-48 steppenmotor with ULN2003 driver
- M5 Atom light
- 0,9" SSD1306 display (128x64)
- microswitch 8x8mm
- watersensor (DFrobot Gravity 5V contacless liquidlevel sensor or DFrobot 3-5v flexible capacative sensor)
- 3D printer parts 

28BYJ-48 met ULN2003 op Atom Lite OLED on grove
print layout:
XX      [GND] [21]                  ← microswitch 
XXX     [GND] [5V] [25]             ← watersensor 5v
XX      [GND] [5V]                  ← stappen 5v

XXXX    [GND] [5V] [25] [21]        ← bovenste 4-pins header
XXXXXX  [33]  [23] [19] [22] [3V3]  ← onderste 5-pins header 

XXXX    [33]  [23] [19] [22] [5V]   ← in1 in2 in3 in4 stappenmotor
*/
#define   pinswap  true      // latest version STOPSWITCH_PIN  21=>25 WATERSENSOR_PIN 25->21
#include  "M5Atom.h"
#include  <Adafruit_SSD1306.h>
#define   SCREEN_WIDTH    128
#define   SCREEN_HEIGHT   64
#define   OLED_RESET      -1
#define   OLED_ADDR       0x3C
Adafruit_SSD1306 display  (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#ifdef pinswap
  #define WATERSENSOR_PIN 25
  #define STOPSWITCH_PIN  21
#else
  #define WATERSENSOR_PIN 21
  #define STOPSWITCH_PIN  25
#endif
#define   STEPS_PER_REV   4096
#define   M5BUTTON_PIN    39
#define   multistep       8
#define   maxmil          80
const int  motorPins[4] = {33, 23, 19, 22}; // IN1–IN4
const int stepSeq[8][4] = {
  {1,0,0,0},
  {1,1,0,0},
  {0,1,0,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,1},
  {0,0,0,1},
  {1,0,0,1}
};
int     afstand,afstandoud,flip,homing_count;
int     max_homing_steps =  800; //80mm=800
bool    nowater,button,mswitch,longpress;
String  action;
// kalibratie constants
const   float start_offset_mm   = 46.0;   // mm positie startmeetpunt
const   float height_1liter_mm  = 90.0;   // mm hoogte komt overeen met +1 liter
// opslag van actuele waarden
float   volume = 0.0f;                    // in liter 

void setup() {
  M5.begin              (true, false, true);
  delay                 (50);
  Wire.begin            (32, 26); // SCL = 32, SDA = 26 (grove connector)
  Serial.println        (__FILE__);
  if(!display.begin     (SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println      (F("SSD1306 allocation failed"));
    M5.dis.drawpix      (0, 0xff0000);
  } else {
    M5.dis.drawpix      (0, 0x00ff00);
    Serial.println      ("OLED initialised");
  }
  display.setTextSize   (4);
  display.setTextColor  (SSD1306_WHITE);
  for(int i=0;i<4;i++)  {
  pinMode               (motorPins[i], OUTPUT);}
  pinMode               (WATERSENSOR_PIN, INPUT);       // evt. INPUT_PULLUP afhankelijk van sensor
  pinMode               (M5BUTTON_PIN, INPUT); 
  pinMode               (STOPSWITCH_PIN, INPUT_PULLUP); // schakelaar naar GND 
  readButton            ();                             // M5 button pressed at startup
  if (button==LOW)      {                               // move steppenmotor 400mm away from mswitch;
    multiMotor          (-1,400);
  }
  afstand     =         0;
  startpos              ();
  Serial.println        ("Klaar om te meten!\n");
}

void loop() {
  readButton();             // M5 button pressed
  if (button==LOW)          {
    if (longpress){
      startpos();
    }else{
      measure();}
  }
  if (afstandoud!=afstand)  {Serial.println(String (afstand)+ " mm.");afstandoud=afstand;}
  delay(500);
}

void direct_start()         {
  action              =     "to start.";
  int stappen_omlaag  =     afstand * 10;
  if (stappen_omlaag > 0)   {
    Serial.println          (String(stappen_omlaag)+" stappen naar beneden = "+String(stappen_omlaag/10));
    multiMotor              (-1, stappen_omlaag);         // Beweeg naar beneden
    afstand           =     0;                            // Zet afstand op startmeetpunt
    Serial.println          ("Direct naar startmeetpunt (afstand=0)");
  } else {
    Serial.println          ("Reeds op of onder startmeetpunt!");
  }
}

void startpos(){
  action              =   "homing...";
  int     failsafe    =   max_homing_steps;
  // move furtest away from startpos (not adjusting afstand)
  while   (digitalRead    (STOPSWITCH_PIN) == HIGH) {
    stepMotor             (1, multistep);    // langzaam naar schakelaar toe
    failsafe--;
    if (failsafe<0)       {break;}
  }
  if                      (digitalRead(STOPSWITCH_PIN)==LOW){
    action        =       "to start.";
    afstand       =       max_homing_steps/10;//80
    multiMotor            (-1,max_homing_steps);//adjusting afstand
    homing_count  =       max_homing_steps;
    Serial.println        ("adjust max_homing_steps if still not at endpoint > "+String(max_homing_steps));
  } else                  {Serial.println ("home niet gevonden!");}
  action          =       "press BTN";
  oled                    (afstand);
}

void measure(){
  Serial.println          ("measure waterlevel");
  action          =       "**WAIT**";
  oled                    (afstand);
  for(int i=0;i<5;i++)    {
    M5.dis.drawpix        (0, 0xff0000);
    delay (500);
    M5.dis.drawpix        (0, 0x000000);
    delay (500);
  }
  readWater();
  if        (nowater){
    Serial.print     ("NO water at current level");
    int     option=0;
    switch  (option) {
    case 0:  // option0 search down untill water
      Serial.println(" search_down");
      search_do   ();	
      search_up   ();
      break;
    case 1:  // option1 move to start position the long way (full up then full down)
      Serial.println(" full up then full down then search_up");
      startpos    (); 
      search_up   ();	
      break;
    case 2:  // option2 move direct to start position 
      Serial.println(" full down then search_up");
      direct_start();
      search_up   ();	
      break;
    case 3:  // option3 do 10 steps back then search up again
      Serial.println(" 10 mm. down then search_up");
      if (afstand>=10){multiMotor(-1,100);}
      search_up   ();	
      break;
    }
  } else { //water
      Serial.print  ("Water at current level!");
      Serial.println(" search_up");
      search_up     ();  
  }
}

void search_up(){
//search up
  action        =         "searching";
  oled                    (afstand);
    while (!nowater){
    if (afstand>max_homing_steps/10){break;}
    multiMotor            (1,10);//10=1mm
    readWater             ();
    oled                  (afstand);
    Serial.print          (afstand); 
    Serial.print          ("mm. / ");
    Serial.print          (volume);
    Serial.println        ("ltr.");
  }
  if (afstand>max_homing_steps/10)          {
    action        =       "** FULL **";
    oled                  (afstand);
  }else{
    action        =       "waterlevel";
    oled                  (afstand);
  }
}

void search_do(){
//search down
  action        =         "srchdown";
  oled                    (afstand);
    while (nowater){
    if (afstand<0)        {break;}
    multiMotor            (-1,10);//10=1mm
    readWater             ();
    oled                  (afstand);
    Serial.print          (afstand); 
    Serial.print          ("mm. / ");
    Serial.print          (volume);
    Serial.println        ("ltr.");
  }
  if (afstand<0)          {
    action        =       "**EMPTY**";
    oled                  (afstand);
  }else{
    action        =       "waterlevel";
    oled                  (afstand);
  }
}

void readButton(){        // lees button
  longpress =             false;
  button    =             digitalRead(M5BUTTON_PIN); 
  if (button==LOW)      {                               // move steppenmotor 400mm away from mswitch;
    int cnt=0;
    while (digitalRead(M5BUTTON_PIN)==LOW) {delay(100);cnt++;}
    if (cnt>20){longpress=true;}else{longpress=false;}
  }
}

void readWater(){         // Lees watersensor
  nowater    =            digitalRead(WATERSENSOR_PIN); // afhankelijk van sensor: HIGH of LOW bij nat
//Serial.print            ("Water: "); Serial.println(nowater ? "NEE" : "JA");
  if (nowater)            {M5.dis.drawpix(0,0xffffff );}else{M5.dis.drawpix(0,0x0000ff);}
}
/*
void homing()             {  // naar referentiepunt
  Serial.print            ("Begin homing...");
  action          =       "homing...";
  homing_count    =       0;
  while(digitalRead       (STOPSWITCH_PIN) == HIGH && homing_count < max_homing_steps) {
    stepMotor             (1, multistep);    // langzaam naar schakelaar toe
    flip++;if (flip==10)  {afstand++;flip=0;oled(afstand);}
    if (first)            {homing_count++;}
  }
  Serial.println          (digitalRead(STOPSWITCH_PIN)==LOW ? "gevonden!" : "niet gevonden!");
  Serial.println          ("homing_count:"+String(homing_count));
}
*/
void oled(int waarde){
  display.clearDisplay    ();
  display.setTextSize     (2);
  display.setCursor       (0,0);
  display.println         (action);
  display.setTextSize     (3);
  display.print           (afstand); 
  display.println         ("mm.");
  // volume = offset in liters + extra hoogte in liters
  volume    =             (start_offset_mm / height_1liter_mm) + (afstand / height_1liter_mm);
  if (volume < 0.0f)      volume = 0.0f;
  display.print           (volume);
  display.println         ("lt.");
  display.display         ();
  readWater               ();
}

void multiMotor           (int mdir, int msteps) {
  flip=0;
  for (int m=0;m<msteps;m++){
  //if (digitalRead       (M5BUTTON_PIN)==LOW){break;}
    flip++;
    if    (flip==10){
      if  (mdir>0){
          afstand++;
      }else{
          afstand--;
      }
      flip=0;oled(afstand);
    }
    if (afstand<0){break;}
    if (afstand>max_homing_steps/10){break;}
    stepMotor(mdir,multistep);
  }
}

void stepMotor(int dir, int steps) {//no afstand count
  for   (int i=0;i<steps;i++) {
    int idx = dir>0 ? i%8 : (7-(i%8)); // vooruit/achteruit
    for (int j=0;j<4;j++) 
        digitalWrite  (motorPins[j], stepSeq[idx][j]);
        delay         (2); // pas eventueel snelheid aan
  }
  // Motor uit
  for   (int j=0;j<4;j++){
        digitalWrite  (motorPins[j],LOW);
  }
}
