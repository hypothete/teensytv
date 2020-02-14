#define PIN_A 14
#define PIN_B 15
#define PIN_C 16
#define PIN_D 17

#define WIDTH 52
#define HEIGHT 242
#define SWIDTH 64
#define SHEIGHT 262
#define SLENGTH 16700 // a little fudged

uint8_t screenBuffer[SLENGTH];
int screenStart = SWIDTH * 20 + 10;
volatile int usbPosition = screenStart;
volatile int frameCount = 0;

uint8_t vsync(int x, int y) {
  if (y < 9) {
    // 9 pulse lines:
    // 6 pre equalizing pulses
    // 6 sync pulses
    // 6 post equalizing pulses
    int pulse = y/3 % 2;
    int pulseDelay = pulse ? 4 : 2;
    int hx = x / 2;
    if (hx < pulseDelay) {
      return pulse ? 1 : 0;
    }
    else {
      return pulse ? 0 : 1;
    }
  }
  else {
    // 11 black lines
    return x < 5 ? 0 : 1;
  }
}

uint8_t halfline(int x) {
  return x < 6 ? 1 : 0;
}

uint8_t scanline(int x, int y) {
  if (x < 5) {
    // sync
    return 0;
  }
  else if(x < 10) {
    // back porch
    return 1;
  }
  else if(x < 62) {
    return drawBuffer(x, y);
  }
  else {
    return 1;
  }
}

void setupBuffer() {
  for(int i=0; i<SLENGTH; i++) {
    int x = i % SWIDTH;
    int y = i / SWIDTH;
    if (y < 20) {
      screenBuffer[i] = vsync(x, y);
    }
    else if(y < SHEIGHT-1) {
      screenBuffer[i] = scanline(x, y);
    }
    else {
      screenBuffer[i] = halfline(x);
    }
  }
}

uint8_t drawBuffer(int x, int y) {
  // x value starts at sample 10, y value starts at line 20
  int i = x - 10;
  int j = y - 20;
  uint8_t px = (i % 16) ^ (j % 16); // screenBuffer[i + j * WIDTH];
  return max(1,px);
}

void writeComposite(uint8_t v) {
  // tied to schematic, consult core_pins.h
  uint32_t ab = (0x3 & v) << 18;
  uint32_t c = (0x4 & v) << 21;
  uint32_t d = (0x8 & v) << 19;
  GPIO6_DR = (GPIO6_DR & ~13369344) | ab | c | d;
}

void writePixel(int x, int y, int p) {
  int i = x + 10;
  int j = y + 20;
  screenBuffer[i + j * SWIDTH] = max(p, 1);
}

void readRawHID() {
  if (RawHID.available()) {
    byte incomingBuffer[64];
    int responseLength = RawHID.recv(incomingBuffer, 0);
    for (int i=0; i<responseLength; i++) {
      screenBuffer[usbPosition] = max(incomingBuffer[i], 1);
      usbPosition++;
      int x = usbPosition % SWIDTH;
      if (x > SWIDTH-2) {
        usbPosition += 10 + 2;
        if(usbPosition >= SLENGTH - SWIDTH) {
          usbPosition = screenStart;
          i+= 25;
        }
      }
    }
  }
}

void setup() {
  pinMode(PIN_A,  OUTPUT);
  pinMode(PIN_B,  OUTPUT);
  pinMode(PIN_C,  OUTPUT);
  pinMode(PIN_D,  OUTPUT);
  setupBuffer();
}

void loop() {
  elapsedMicros ts;
  while(1) {
    if(ts == 0) {
      readRawHID();
    }
    writeComposite(screenBuffer[ts]);
    frameCount++;
    if (ts >= SLENGTH) {
      ts = 0;
    }
  }
}
