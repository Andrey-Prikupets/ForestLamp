#include <FastLED.h>

FASTLED_USING_NAMESPACE


#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define POT_PIN     A1
#define OFF_PIN     3

#define DATA_PIN    2
#define LED_TYPE    WS2812

#define WIDTH                 (8U)  // ширина матрицы
#define HEIGHT                (11U) // высота матрицы

#define COLOR_ORDER           (GRB) // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE           (0U)  // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE      (0U)  // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION       (1U)  // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
                                    // при неправильной настройке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
                                    // шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/
#define NUM_LEDS              (uint16_t)(WIDTH * HEIGHT)
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          25 // 0-255
#define FRAMES_PER_SECOND  120

#define NUM_READINGS          (10)

#define SEGMENTS              (1U)                          // диодов в одном "пикселе" (для создания матрицы из кусков ленты)

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

bool loadingFlag = true;
unsigned char matrixValue[8][16];

// Scale in range 1..100;
// Delay in ms;
#define FIRE_SCALE        10 // yellow fire;
#define FIRE_DELAY        10

#define RAINBOW_VER_SCALE 25 // 10
#define RAINBOW_VER_DELAY 100

#define RAINBOW_HOR_SCALE 10
#define RAINBOW_HOR_DELAY 100

#define RAINBOW_DIAG_SCALE 40
#define RAINBOW_DIAG_DELAY 100

#define MATRIX_SCALE      50
#define MATRIX_DELAY      100

static const uint8_t maxDim = max(WIDTH, HEIGHT);

// START; Effects from FastLed's DemoReel;

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

// END; Effects from FastLed's DemoReel;

// START; Effects from AlexGyver's WiFi Lamp;

// служебные функции

// залить все
void fillAll(CRGB color)
{
  for (int32_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
}

// функция отрисовки точки по координатам X Y
void drawPixelXY(int16_t x, int16_t y, CRGB color)
{
  if (x < 0 || x > (int16_t)(WIDTH - 1) || y < 0 || y > (int16_t)(HEIGHT - 1)) return;
  uint32_t thisPixel = getPixelNumber((uint8_t)x, (uint8_t)y) * SEGMENTS;
  for (uint8_t i = 0; i < SEGMENTS; i++)
  {
    leds[thisPixel + i] = color;
  }
}

// функция получения цвета пикселя по его номеру
uint32_t getPixColor(uint32_t thisSegm)
{
  uint32_t thisPixel = thisSegm * SEGMENTS;
  if (thisPixel > NUM_LEDS - 1) return 0;
  return (((uint32_t)leds[thisPixel].r << 16) | ((uint32_t)leds[thisPixel].g << 8 ) | (uint32_t)leds[thisPixel].b);
}

// функция получения цвета пикселя в матрице по его координатам
uint32_t getPixColorXY(uint8_t x, uint8_t y)
{
  return getPixColor(getPixelNumber(x, y));
}

// ************* НАСТРОЙКА МАТРИЦЫ *****
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y x

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y (WIDTH - x - 1)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y y

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y (WIDTH - x - 1)

#else
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"

#endif

// получить номер пикселя в ленте по координатам
uint16_t getPixelNumber(uint8_t x, uint8_t y)
{
  if ((THIS_Y % 2 == 0) || MATRIX_TYPE)                     // если чётная строка
  {
    return (THIS_Y * _WIDTH + THIS_X);
  }
  else                                                      // если нечётная строка
  {
    return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);
  }
}

uint8_t line[WIDTH];
uint8_t pcnt = 0U;
#define SPARKLES              (1U)                          // вылетающие угольки вкл выкл

//these values are substracetd from the generated values to give a shape to the animation
static const uint8_t valueMask[8][16] PROGMEM =
{
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
  {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192},
  {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255},
  {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
static const uint8_t hueMask[8][16] PROGMEM =
{
  {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 }
};

void fireRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
    generateLine();
    memset(matrixValue, 0, sizeof(matrixValue));
  }
  EVERY_N_MILLISECONDS( FIRE_DELAY ) {
    if (pcnt >= 100)
    {
      shiftUp();
      generateLine();
      pcnt = 0;
    }
    drawFrame(pcnt, true);
    pcnt += 30;
  }
}

// Randomly generate the next line (matrix row)
void generateLine()
{
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    line[x] = random(64, 255);
  }
}

void shiftUp()
{
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--)
  {
    for (uint8_t x = 0U; x < WIDTH; x++)
    {
      uint8_t newX = x;
      if (x > 15U) newX = x % 16U;
      if (y > 7U) continue;
      matrixValue[y][newX] = matrixValue[y - 1U][newX];
    }
  }

  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    uint8_t newX = x;
    if (x > 15U) newX = x % 16U;
    matrixValue[0U][newX] = line[newX];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation

void drawFrame(uint8_t pcnt, bool isColored)
{
  int32_t nextv;

  //each row interpolates with the one before it
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--)
  {
    for (uint8_t x = 0U; x < WIDTH; x++)
    {
      uint8_t newX = x;
      if (x > 15U) newX = x % 16U;
      if (y < 8U)
      {
        nextv =
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
          - pgm_read_byte(&valueMask[y][newX]);

        CRGB color = CHSV(
          isColored ? FIRE_SCALE * 2.5 + pgm_read_byte(&hueMask[y][newX]) : 0U,     // H
          isColored ? 255U : 0U,                                                               // S
          (uint8_t)max(0, nextv)                                                               // V
        );

        leds[getPixelNumber(x, y)] = color;
      }
      else if (y == 8U && SPARKLES)
      {
        if (random(0, 20) == 0 && getPixColorXY(x, y - 1U) != 0U) drawPixelXY(x, y, getPixColorXY(x, y - 1U));
        else drawPixelXY(x, y, 0U);
      }
      else if (SPARKLES)
      {
        // старая версия для яркости
        if (getPixColorXY(x, y - 1U) > 0U)
          drawPixelXY(x, y, getPixColorXY(x, y - 1U));
        else drawPixelXY(x, y, 0U);
      }
    }
  }

  //first row interpolates with the "next" line
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    uint8_t newX = x;
    if (x > 15U) newX = x % 16U;
    CRGB color = CHSV(
      isColored ? FIRE_SCALE * 2.5 + pgm_read_byte(&(hueMask[0][newX])): 0U,        // H
      isColored ? 255U : 0U,                                                                   // S
      (uint8_t)(((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0)           // V
    );
    //leds[getPixelNumber(newX, 0)] = color;                                         // на форуме пишут что это ошибка - вместо newX должно быть x, иначе
    leds[getPixelNumber(x, 0)] = color;                                              // на матрицах шире 16 столбцов нижний правый угол неработает
  }
}

// ------------- радуга вертикальная ----------------
// uint8_t hue;

#define hue gHue

void rainbowVerticalRoutine()
{
  EVERY_N_MILLISECONDS( RAINBOW_VER_DELAY ) {
    hue += 4;
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      CHSV thisColor = CHSV((uint8_t)(hue + j * RAINBOW_VER_SCALE), 255, 255);
      for (uint8_t i = 0U; i < WIDTH; i++)
      {
        drawPixelXY(i, j, thisColor);
      }
    }
  }
}

// ------------- радуга горизонтальная ----------------
void rainbowHorizontalRoutine()
{
  EVERY_N_MILLISECONDS( RAINBOW_HOR_DELAY ) {
    hue += 4;
    for (uint8_t i = 0U; i < WIDTH; i++)
    {
      CHSV thisColor = CHSV((uint8_t)(hue + i * RAINBOW_HOR_SCALE), 255, 255);
      for (uint8_t j = 0U; j < HEIGHT; j++)
      {
        drawPixelXY(i, j, thisColor);
      }
    }
  }
}

// ------------- радуга диагональная -------------
void rainbowDiagonalRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }

  EVERY_N_MILLISECONDS( RAINBOW_DIAG_DELAY ) {
    hue += 8;
    for (uint8_t i = 0U; i < WIDTH; i++)
    {
      for (uint8_t j = 0U; j < HEIGHT; j++)
      {
        float twirlFactor = 3.0F * (RAINBOW_DIAG_SCALE / 100.0F);      // на сколько оборотов будет закручена матрица, [0..3]
        CRGB thisColor = CHSV((uint8_t)(hue + (float)(WIDTH / HEIGHT * i + j * twirlFactor) * (float)(255 / maxDim)), 255, 255);
        drawPixelXY(i, j, thisColor);
      }
    }
  }
}

// ------------- матрица ---------------
void matrixRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }
  EVERY_N_MILLISECONDS( MATRIX_DELAY ) {
    for (uint8_t x = 0U; x < WIDTH; x++)
    {
      // заполняем случайно верхнюю строку
      uint32_t thisColor = getPixColorXY(x, HEIGHT - 1U);
      if (thisColor == 0U)
        drawPixelXY(x, HEIGHT - 1U, 0x00FF00 * (random(0, 100 - MATRIX_SCALE) == 0U));
      else if (thisColor < 0x002000)
        drawPixelXY(x, HEIGHT - 1U, 0U);
      else
        drawPixelXY(x, HEIGHT - 1U, thisColor - 0x002000);
    }
  
    // сдвигаем всё вниз
    for (uint8_t x = 0U; x < WIDTH; x++)
    {
      for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
      {
        drawPixelXY(x, y, getPixColorXY(x, y + 1U));
      }
    }
  }
}

// END; Effects from AlexGyver's WiFi Lamp;

int readings[NUM_READINGS];     // the readings from the analog input

void setup() {
  Serial.begin(115200);
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(POT_PIN, INPUT); // set pin to input;
  digitalWrite(POT_PIN, LOW); // disable pull-ups;
     
  pinMode(OFF_PIN, INPUT); // set pin to input;
  digitalWrite(OFF_PIN, HIGH); // enable pull-ups;
  
  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = 0;
  }

  memset(matrixValue, 0, sizeof(matrixValue));
  randomSeed(micros());
  loadingFlag = true;
}

int getValueAvg() {
  static int readIndex = 0;              // the index of the current reading
  static int total = 0;                  // the running total
  
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(POT_PIN);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= NUM_READINGS) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  return total / NUM_READINGS;
}

#define HIST 3

int getValueHist() {
  static int prevValue = -1;
  int value = getValueAvg();
  if (prevValue < 0) {
    prevValue = value;
    return value;
  }
  int delta = value-prevValue;
  if (delta < 0)  
    delta = -delta;
  if (delta <= HIST)
    return prevValue;
  value = (prevValue+value)/2;
  prevValue = value;
  return value; 
}

#define ADC_MAX 1023

int getValue() {
  int value = ADC_MAX-getValueHist();
  static int value1 = -1;
  if (value != value1) {
    Serial.print("value="); Serial.println(value,DEC);
    value1 = value;
  }
  return value;  
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
//   rainbow,            
//   juggle,              
//   rainbowWithGlitter, 
//   bpm,
SimplePatternList gPatterns = { 
   confetti,            
   sinelon,             
   fireRoutine,
   rainbowVerticalRoutine,
   rainbowHorizontalRoutine,
   rainbowDiagonalRoutine,
   matrixRoutine
  };              

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define POT_MAX ADC_MAX
#define POT_WHITE_MIN 0
#define POT_WHITE_MAX 15
#define POT_COLORS_MIN POT_WHITE_MAX+1
#define POT_COLORS_MAX (POT_MAX/4)
#define EFFECTS_MIN (POT_COLORS_MAX+1)
#define EFFECTS_MAX (POT_MAX-10)
#define DEMO_MIN (EFFECTS_MAX+1)
#define DEMO_MAX POT_MAX
#define EFFECTS_RANGE (EFFECTS_MAX-EFFECTS_MIN+1)
#define EFFECTS_NUM ARRAY_SIZE(gPatterns)
#define EFFECTS_STEP (EFFECTS_RANGE/EFFECTS_NUM)

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  Serial.print(F("Pattern ")); Serial.println(gCurrentPatternNumber, DEC);
}

void setAllLeds(CHSV color) {
  CRGB col = (CRGB) color;
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = col;
  }  
}

inline bool inRange(int value, int minValue, int maxValue) {
  return value >= minValue && value <= maxValue;
}

bool isSwitched(int pos) {
  static int pos1 = -1;
  if (pos != pos1) {
    Serial.print(F("Triggered to ")); Serial.println(pos, DEC);
    pos1 = pos;
    return true;
  }
  return false;
}

void loop()
{
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  if (digitalRead(OFF_PIN)) {
    FastLED.clear();
    FastLED.show();  
    return;
  }
  
  int value = getValue();
  if (inRange(value, POT_WHITE_MIN, POT_WHITE_MAX)) {
    isSwitched(POT_WHITE_MIN);
    setAllLeds(CHSV(0, 0, 255));
  } else
  if (inRange(value, POT_COLORS_MIN, POT_COLORS_MAX)) {
    isSwitched(POT_COLORS_MIN);
    int hue = constrain(map(value,POT_COLORS_MIN,POT_COLORS_MAX,0,255), 0, 255);
    setAllLeds(CHSV(hue, 255, 255));
  } else
  if (inRange(value, EFFECTS_MIN, EFFECTS_MAX)) {
    int effect = map(value,EFFECTS_MIN,EFFECTS_MAX,0,EFFECTS_NUM);
    #define ARTIFICIAL_EFFECTS_HASH_BASE 10000
    if (isSwitched(ARTIFICIAL_EFFECTS_HASH_BASE+effect)) {
      loadingFlag = true;  
      gHue = 0;
    }
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    gPatterns[effect]();
  } else
  if (inRange(value, DEMO_MIN, DEMO_MAX)) {
    if (isSwitched(DEMO_MIN)) {
      gCurrentPatternNumber = 0;
      gHue = 0;
    }
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();
    
    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically    
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();    
}
