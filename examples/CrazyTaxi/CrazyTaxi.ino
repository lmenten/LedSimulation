// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.

#include <TouchScreen.h>
#include <LedSimulation.h>

// default pins
#define btnLeft   3
#define btnRight  2

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Adafruit_TFTLCD tft;

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
LedSimulation *led = new LedSimulation(tft);

unsigned long LastTime;
unsigned long LastTimeMoved;

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Bit Race"));

  tft.reset();

  uint16_t identifier = tft.readID();

  if (identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if (identifier == 0x9327) {
    Serial.println(F("Found ILI9327 LCD driver"));
  } else if (identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if (identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if (identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if (identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if (identifier == 0x0154) {
    Serial.println(F("Found S6D0154 LCD driver"));
  } else if (identifier == 0x9488) {
    Serial.println(F("Found ILI9488 LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    return;
  }

  tft.begin(identifier);

  Serial.println(F("Game!"));
  tft.fillScreen(BLACK);
  tft.fillRect(40, 320 - 40, 40, 40, BLUE);
  tft.fillRect(40 * 4, 320 - 40, 40, 40, BLUE);

  long LastTime = 0;
  long LastTimeMoved = 0;

  pinMode(13, OUTPUT);
}

bool isHome = true; // display the homescreen
unsigned long deadTime = millis(); // time that the player dies
int heroX = 5; //start the hero in the center of the screen
float vSpeed = 0.3;
float borderPos = 0;
bool pressed = false; //indicates if there is a button pressed
// positions of the enemies
int enemyX[2] = {5, 10};
float enemyY[2] = {0, -8};

/**
   Updates the player position
   @param  {int} int d             player direction. 0 keep, -1 left, 1 right
*/
void goDirection(int d) {
  if (d > 0 && heroX < 12) {
    Serial.println("GoLeft");
    heroX += d;
  } else if (d < 0 && heroX > 3) {
    Serial.println("GoRight");
    heroX += d;
  }
}

/**
   Draws the hero car in the screen
   Hero looks like:
 *         *
 *        ***
 *         *
 *        ***
    HeroX indicates the middle px in the bottom of the car
*/
void drawHero(void)
{
  led->setLedOn(15, heroX);
  led->setLedOn(15, heroX + 1);
  led->setLedOn(15, heroX - 1);
  led->setLedOn(14, heroX);
  led->setLedOn(13, heroX);
  led->setLedOn(13, heroX + 1);
  led->setLedOn(13, heroX - 1);
  led->setLedOn(12, heroX);
}

/**
   Draw road edges
*/
void drawBorder(void)
{
  byte i;
  if (borderPos > 1) {
    i = 1;
  } else {
    i = 0;
  }
  if (borderPos > 2)
    borderPos = 0;
  for (i; i < 16; i += 2) {
    led->setLedOn(i, 1);
    led->setLedOn(i, 14);
  }
}

/**
   Update entities positions in the scene
*/
void updatePos(void)
{
  borderPos += vSpeed;

  for (byte e = 0; e < 2; e++) {
    enemyY[e] += vSpeed;
    if (enemyY[e] > 18) {
      enemyY[e] = -1;
      if (random(2))
        enemyX[e] = 5;
      else
        enemyX[e] = 10;
    }
  }
}

/**
   restore default values
*/
void restartGame(void)
{
  heroX = 5;
  borderPos = 0;
  pressed = false;
  enemyX[0] = 5;
  enemyX[1] = 10;
  enemyY[0] = 0;
  enemyY[1] = -8;
}

/** Dies */
void heroDie(void)
{
  deadTime = millis();
  isHome = true;
  restartGame();
}

void checkCollisions() {
  for (byte e = 0; e < 2; e++) {
    int ex = enemyX[e], ey = (int)enemyY[e];
    int diffX = abs(heroX - ex);

    if (ey >= 13 && diffX <= 2) {
      heroDie();
    }
  }
}

/** Draw enemies in the scene
  Enemy looks like:

         ***
         ***

  The position of the enemy indicates the * in the bottom
*/
void drawEnemies() {
  for (byte e = 0; e < 2; e++) {
    byte y = (byte) enemyY[e];
    byte x = (byte) enemyX[e];

    if (y >= 0 && y <= 15)
      led->setLedOn(y, x);
    if (y - 1 >= 0 && y - 1 <= 15) {
      led->setLedOn(y - 1, x);
      led->setLedOn(y - 1, x - 1);
      led->setLedOn(y - 1, x + 1);
    }
    if (y - 2 >= 0 && y - 2 <= 15) {
      led->setLedOn(y - 2, x - 1);
      led->setLedOn(y - 2, x + 1);
      led->setLedOn(y - 2, x);
    }
  }
}

/**
   Check collisions and update game entities
*/
void gameUpdate() {
  //Serial.println("Update");
  updatePos();
  checkCollisions();
  drawHero();
  drawEnemies();
  drawBorder();
  led->draw();
}

unsigned char  Logo[16][16] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

unsigned char  Home[16][16] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1},
  {1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1},
  {1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1},
  {1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1},
  {1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1},
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1},
  {1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0},
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1},
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0},
};

/**
   Arduino loop. If game is running, just keep the game loop running.
   Otherwise shows the homescene
*/
// game variables
bool isLogo = true; // display the homescreen
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940


int touch(int direction)
{
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 320, 0);
    Serial.print("("); Serial.print(p.x);
    Serial.print(", "); Serial.print(p.y);
    Serial.println(")");
    if (p.y > 320 - 40) {
      //Serial.println("A palpable hit!");
      if ((p.x > 0) && (p.x < 80)) {
        Serial.print("Left! p.z: ");
        Serial.println(p.z);
        direction = -1;
      } else if (p.x > (40 * 4)) {
        Serial.println("Right! p.z: ");
        Serial.println(p.z);
        direction = 1;
      }
    }
    //} else {
    //Serial.println("No touch.");
  }
  return direction; //player direction. 0 keep pos, -1 go left, +1 go right
}

int direction = 0;
void loop(void)
{
  unsigned long CurTime = millis();
  direction = touch(direction);

  if (isLogo) {
    led->setScene(Logo);
    led->draw();
    if (millis() - deadTime > 5000)
      isLogo = false;
  } else {
    if (isHome) {
      led->setScene(Home);
      led->draw();
      //if (direction != 0 && millis() - deadTime > 1000)
      if (millis() - deadTime > 1000)
        isHome = false;
    } else {
      if ((CurTime - LastTimeMoved) > 5) {
        goDirection(direction);
        direction = 0;
        LastTimeMoved = CurTime;
      }
      if ((CurTime - LastTime) > 30) {
        gameUpdate();
        LastTime = CurTime;
      }
    }
  }
}
