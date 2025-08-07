//PFD AHI V3 with sprite integration
// Written for a 320x240 LCD tft ST7789 display

#include <Wire.h>
#include <TFT_eSPI.h> // Use TFT_eSPI library for ST7789


TFT_eSPI tft = TFT_eSPI(); // Create TFT object

// --- Pin Definitions & Color Constants ---
#define TFT_CS    4
#define TFT_RST   19
#define TFT_DC    5
#define TFT_SCLK  18
#define TFT_MOSI  23

#define REDRAW_DELAY 16 // minimum delay in milliseconds between display updates

#define HOR 240  // Adjusted for a wider screen

#define BROWN      0x5140 //0x5960
#define SKY_BLUE   0x02B5 //0x0318 //0x039B //0x34BF
#define DARK_RED   0x8000
#define DARK_GREY  0x39C7

#define XC 160  // Half of 320
#define YC 120  // Half of 240

#define DEG2RAD 0.0174532925

int last_roll = 0; // the whole horizon graphic
int last_pitch = 0;

// Variables for test only
int test_roll = 0;
int delta = 0;

unsigned long redrawTime = 0;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 50;  // Update every 50ms

void setup(void) {
  Serial.begin(115200);

  tft.init();

  //tft.initDMA();  // Initialize DMA  great for performance on graphics updates
  tft.setRotation(1);
  tft.fillRect(0,  0, 320, 120, SKY_BLUE);
  tft.fillRect(0, 120, 320, 120, BROWN);

   //horizonSprite.createSprite(320, 240);  // Create a sprite of screen size

int pitch = 0;
int roll = 0;
  // Draw the horizon graphic
  drawHorizon(0, 0);
  drawInfo(roll, pitch);
  delay(200); // Wait to permit visual check

  // Test roll and pitch
  testRoll();
  testPitch();

  tft.setTextColor(TFT_YELLOW, SKY_BLUE);
  tft.setTextDatum(TC_DATUM);            // Centre middle justified
  tft.drawString("Random", 64, 10, 1);
}

void loop() {

  // Refresh the display at regular intervals
  if (millis() > redrawTime) {
    redrawTime = millis() + REDRAW_DELAY;

    // Roll is in degrees in range +/-180
    int roll = random(361) - 180;

    // Pitch is in y coord (pixel) steps, 20 steps = 10 degrees on drawn scale
    // Maximum pitch shouls be in range +/- 80 with HOR = 172
    int pitch = random (161) - 80;


     // Read pitch and roll from your sensor (Replace with actual data source)
    //int roll = getRollValue();  // Function to get roll value
    //int pitch = getPitchValue();  // Function to get pitch value

     //drawAHI(roll, pitch);  // Update artificial horizon
     drawInfo(roll, pitch);  // Update pitch scale display

    updateHorizon(roll, pitch);
    delay(20);  // Adjust for smooth updates

  }
  
}

// Update the horizon with a new roll (angle in range -180 to +180)

void updateHorizon(int roll, int pitch)
{
  bool draw = 1;
  int delta_pitch = 0;
  int pitch_error = 0;
  int delta_roll  = 0;
  while ((last_pitch != pitch) || (last_roll != roll))
  {
    delta_pitch = 0;
    delta_roll  = 0;

    if (last_pitch < pitch) {
      delta_pitch = 1;
      pitch_error = pitch - last_pitch;
    }

    if (last_pitch > pitch) {
      delta_pitch = -1;
      pitch_error = last_pitch - pitch;
    }

    if (last_roll < roll) delta_roll  = 1;
    if (last_roll > roll) delta_roll  = -1;

    if (delta_roll == 0) {
      if (pitch_error > 1) delta_pitch *= 2;
    }

    drawHorizon(last_roll + delta_roll, last_pitch + delta_pitch);
    drawInfo(roll, pitch);
  }
}

// Draw the horizon with a new roll (angle in range -180 to +180)

void drawHorizon(int roll, int pitch)
{
  // Calculate coordinates for line start
  float sx = cos(roll * DEG2RAD);
  float sy = sin(roll * DEG2RAD);

  int16_t x0 = sx * HOR;
  int16_t y0 = sy * HOR;
  int16_t xd = 0;
  int16_t yd = 1;
  int16_t xdn  = 0;
  int16_t ydn = 0;

  if (roll > 45 && roll <  135) {
    xd = -1;
    yd =  0;
  }
  if (roll >=  135)             {
    xd =  0;
    yd = -1;
  }
  if (roll < -45 && roll > -135) {
    xd =  1;
    yd =  0;
  }
  if (roll <= -135)             {
    xd =  0;
    yd = -1;
  }

  if ((roll != last_roll) && ((abs(roll) > 35)  || (pitch != last_pitch)))
  {
    xdn = 4 * xd;
    ydn = 4 * yd;
    tft.drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE);
    tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);
    xdn = 3 * xd;
    ydn = 3 * yd;
    tft.drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE);
    tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);
  }
  xdn = 2 * xd;
  ydn = 2 * yd;
  tft.drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE);
  tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);

  tft.drawLine(XC - x0 - xd, YC - y0 - yd - pitch, XC + x0 - xd, YC + y0 - yd - pitch, SKY_BLUE);
  tft.drawLine(XC - x0 + xd, YC - y0 + yd - pitch, XC + x0 + xd, YC + y0 + yd - pitch, BROWN);

  tft.drawLine(XC - x0, YC - y0 - pitch,   XC + x0, YC + y0 - pitch,   TFT_WHITE);

  last_roll = roll;
  last_pitch = pitch;

}
struct Line {
  int x1, y1, x2, y2;
};

Line previousLines[NUM_LINES];  // Array to store previous line positions

void drawPitchScale(int roll, int pitch) {
  for (int i = 0; i < NUM_LINES; i++) {
    Line currentLine = calculateLinePosition(roll, pitch, i);
    if (linesAreDifferent(previousLines[i], currentLine)) {
      // Erase previous line
      tft.drawLine(previousLines[i].x1, previousLines[i].y1, previousLines[i].x2, previousLines[i].y2, BACKGROUND_COLOR);
      // Draw new line
      tft.drawLine(currentLine.x1, currentLine.y1, currentLine.x2, currentLine.y2, LINE_COLOR);
      // Update previous line
      previousLines[i] = currentLine;
    }
  }
}
/*void drawPitchScale(int roll, int pitch) {
  for (int i = -40; i <= 40; i += 10) { // Draw every 10 pixels
    int y_offset = i; // Base vertical offset

    // Apply rotation to tilt the scale with the horizon
    int x1 = XC - (12 * cos(roll * DEG2RAD)) + (y_offset * sin(roll * DEG2RAD));
    int y1 = YC - (y_offset * cos(roll * DEG2RAD)) - (12 * sin(roll * DEG2RAD)) - pitch;

    int x2 = XC + (12 * cos(roll * DEG2RAD)) + (y_offset * sin(roll * DEG2RAD));
    int y2 = YC - (y_offset * cos(roll * DEG2RAD)) + (12 * sin(roll * DEG2RAD)) - pitch;

    tft.drawLine(x1, y1, x2, y2, TFT_WHITE);
  }
}
*/


// Draw the information
void drawInfo(int roll, int pitch) {
  int bgColor;
  int pitchOffset = pitch;  // Use pitch to shift the scale up/down

  // Clear previous pitch scale lines
  for (int i = -40; i <= 40; i += 10) {
    bgColor = (i < 0) ? SKY_BLUE : BROWN;  // Choose background color

    tft.drawFastHLine(XC - 12,   YC + i - pitchOffset, 24, bgColor);
    tft.drawFastHLine(XC -  6,   YC + i + 10 - pitchOffset, 12, bgColor);
  }

  // Redraw level wings (unchanged)
  tft.fillRect(XC - 1, YC - 1, 3, 3, TFT_RED);
  tft.drawFastHLine(XC - 30,   YC, 24, TFT_RED);
  tft.drawFastHLine(XC + 30 - 24, YC, 24, TFT_RED);
  tft.drawFastVLine(XC - 30 + 24, YC, 3, TFT_RED);
  tft.drawFastVLine(XC + 30 - 24, YC, 3, TFT_RED);

  // Redraw the pitch scale with movement
  for (int i = -40; i <= 40; i += 10) {
    tft.drawFastHLine(XC - 12,   YC + i - pitchOffset, 24, TFT_WHITE);
    tft.drawFastHLine(XC -  6,   YC + i + 10 - pitchOffset, 12, TFT_WHITE);
  }

  // Redraw pitch scale numbers at the new shifted positions
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(XC - 12 - 13, YC - 20 - 3 - pitchOffset);
  tft.print("10");
  tft.setCursor(XC + 12 + 1, YC - 20 - 3 - pitchOffset);
  tft.print("10");
  tft.setCursor(XC - 12 - 13, YC + 20 - 3 - pitchOffset);
  tft.print("10");
  tft.setCursor(XC + 12 + 1, YC + 20 - 3 - pitchOffset);
  tft.print("10");

  tft.setCursor(XC - 12 - 13, YC - 40 - 3 - pitchOffset);
  tft.print("20");
  tft.setCursor(XC + 12 + 1, YC - 40 - 3 - pitchOffset);
  tft.print("20");
  tft.setCursor(XC - 12 - 13, YC + 40 - 3 - pitchOffset);
  tft.print("20");
  tft.setCursor(XC + 12 + 1, YC + 40 - 3 - pitchOffset);
  tft.print("20");

  // Update roll value display
  tft.setTextColor(TFT_YELLOW, BROWN);
  tft.setTextDatum(MC_DATUM);
  tft.setTextPadding(24);
  tft.drawNumber(roll, XC, YC + 10, 1);

  // Redraw fixed text
  tft.setTextColor(TFT_YELLOW);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("SPD      ALT", XC, 5, 1);
  tft.drawString("Tarcy's AHI", XC, YC + 70, 1);
}


// Function to generate roll angles for testing only

int rollGenerator(int maxroll)
{
  // Synthesize a smooth +/- 50 degree roll value for testing
  delta++; if (delta >= 360) test_roll = 0;
  test_roll = (maxroll + 1) * sin((delta) * DEG2RAD);

  // Clip value so we hold roll near peak
  if (test_roll >  maxroll) test_roll =  maxroll;
  if (test_roll < -maxroll) test_roll = -maxroll;

  return test_roll;
}

// Function to generate roll angles for testing only

void testRoll(void)
{
  tft.setTextColor(TFT_YELLOW, SKY_BLUE);
  tft.setTextDatum(TC_DATUM);            // Centre middle justified
  tft.drawString("Roll test", 64, 10, 1);

  for (int a = 0; a < 360; a++) {
    //delay(REDRAW_DELAY / 2);
    updateHorizon(rollGenerator(180), 0);
  }
  tft.setTextColor(TFT_YELLOW, SKY_BLUE);
  tft.setTextDatum(TC_DATUM);            // Centre middle justified
  tft.drawString("         ", 64, 10, 1);
}

// Function to generate pitch angles for testing only

void testPitch(void)
{
  tft.setTextColor(TFT_YELLOW, SKY_BLUE);
  tft.setTextDatum(TC_DATUM);            // Centre middle justified
  tft.drawString("Pitch test", 64, 10, 1);

  for (int p = 0; p > -80; p--) {
    delay(REDRAW_DELAY / 2);
    updateHorizon(0, p);
  }

  for (int p = -80; p < 80; p++) {
    delay(REDRAW_DELAY / 2);
    updateHorizon(0, p);
  }

  for (int p = 80; p > 0; p--) {
    delay(REDRAW_DELAY / 2);
    updateHorizon(0, p);
  }

  tft.setTextColor(TFT_YELLOW, SKY_BLUE);
  tft.setTextDatum(TC_DATUM);            // Centre middle justified
  tft.drawString("          ", 64, 10, 1);
}