/*
  ============================================================
  Robot Face Expression Controller
  ============================================================
  Library  : U8g2
  Display  : OLED 128x64 I2C (SSD1306)
  Trigger  : Serial.read()

  PERINTAH SERIAL (baud 9600):
    'i' -> IDLE (loop terus)
    'a' -> ANGRY  (main 1x lalu idle)
    'h' -> HAPPY  (main 1x lalu idle)
    's' -> SAD    (main 1x lalu idle)
    'k' -> KAGET  (main 1x lalu idle)
  ============================================================
*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "expressions.h"

// Ganti jika pakai SH1106:
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(...)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ---- TIMING ----
#define FRAME_INTERVAL_MS     100   // 10 FPS
#define EXPRESSION_TIMEOUT_MS 3000  // kembali idle setelah 3 detik

// ---- JUMLAH FRAME (sesuai expressions.h) ----
#define IDLE_FRAMES_TOTAL      25
#define ANGRY_FRAMES_TOTAL     21
#define HAPPY_FRAMES_TOTAL     21
#define SAD_FRAMES_TOTAL       21
#define SURPRISED_FRAMES_TOTAL 29

enum Expression { EXPR_IDLE, EXPR_ANGRY, EXPR_HAPPY, EXPR_SAD, EXPR_SURPRISED };

Expression currentExpression   = EXPR_IDLE;
uint8_t    currentFrame        = 0;
uint32_t   lastFrameTime       = 0;
uint32_t   expressionStartTime = 0;
bool       animationDone       = false;
bool       isLooping           = true;

// ---- Ambil pointer frame ----
const unsigned char* getFrame(Expression expr, uint8_t idx) {
  switch (expr) {
    case EXPR_ANGRY:     return angry_frames[idx];
    case EXPR_HAPPY:     return happy_frames[idx];
    case EXPR_SAD:       return sad_frames[idx];
    case EXPR_SURPRISED: return surprised_frames[idx];
    default:             return idle_frames[idx];
  }
}

// ---- Ambil total frame ----
uint8_t getFrameCount(Expression expr) {
  switch (expr) {
    case EXPR_ANGRY:     return ANGRY_FRAMES_TOTAL;
    case EXPR_HAPPY:     return HAPPY_FRAMES_TOTAL;
    case EXPR_SAD:       return SAD_FRAMES_TOTAL;
    case EXPR_SURPRISED: return SURPRISED_FRAMES_TOTAL;
    default:             return IDLE_FRAMES_TOTAL;
  }
}

// ---- Set ekspresi baru ----
void setExpression(Expression newExpr, bool loop) {
  currentExpression   = newExpr;
  currentFrame        = 0;
  animationDone       = false;
  isLooping           = loop;
  expressionStartTime = millis();
  Serial.print(F("[INFO] Ekspresi: "));
  const char* names[] = {"IDLE","ANGRY","HAPPY","SAD","SURPRISED"};
  Serial.println(names[newExpr]);
}

// ---- Baca Serial ----
void handleSerial() {
  if (!Serial.available()) return;
  char cmd = Serial.read();
  while (Serial.available()) Serial.read();
  switch (cmd) {
    case 'i': case 'I': setExpression(EXPR_IDLE,      true);  break;
    case 'a': case 'A': setExpression(EXPR_ANGRY,     false); break;
    case 'h': case 'H': setExpression(EXPR_HAPPY,     false); break;
    case 's': case 'S': setExpression(EXPR_SAD,       false); break;
    case 'k': case 'K': setExpression(EXPR_SURPRISED, false); break;
    default:
      Serial.println(F("Perintah: i=idle  a=angry  h=happy  s=sad  k=kaget"));
  }
}

// ---- Update animasi ----
void updateAnimation() {
  uint32_t now = millis();
  if (now - lastFrameTime < FRAME_INTERVAL_MS) return;
  lastFrameTime = now;

  uint8_t total = getFrameCount(currentExpression);
  u8g2.clearBuffer();
 u8g2.drawBitmap(0, 0, 16, 64, getFrame(currentExpression, currentFrame));
  u8g2.sendBuffer();

  currentFrame++;
  if (currentFrame >= total) {
    if (isLooping) {
      currentFrame = 0;
    } else {
      currentFrame  = total - 1;
      animationDone = true;
    }
  }

  if (animationDone && (now - expressionStartTime >= EXPRESSION_TIMEOUT_MS)) {
    setExpression(EXPR_IDLE, true);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("=== Robot Face Ready ==="));
  Serial.println(F("i=idle  a=angry  h=happy  s=sad  k=kaget"));
  u8g2.begin();
  u8g2.setDrawColor(1);
  setExpression(EXPR_IDLE, true);
}

void loop() {
  handleSerial();
  updateAnimation();
}
