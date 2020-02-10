#define PIN_A 14
#define PIN_B 15
#define PIN_C 16
#define PIN_D 17
#define WIDTH 52
#define HEIGHT 242
#define VHEIGHT 39

uint8_t screenBuffer[WIDTH * VHEIGHT];
volatile int usbPosition = 0;
volatile int frameCount = 0;

void setupBuffer() {
  for(int i=0; i< WIDTH * VHEIGHT; i++) {
    int x = i % WIDTH;
    int y = i / WIDTH;
    screenBuffer[i] = (x % 16) ^ (y % 16);
  }
}

void writeComposite(uint8_t v) {
  // tied to schematic, consult core_pins.h
  uint32_t ab = (0x3 & v) << 18;
  uint32_t c = (0x4 & v) << 21;
  uint32_t d = (0x8 & v) << 19;
  GPIO6_DR = (GPIO6_DR & ~13369344) | ab | c | d;
}

void readRawHID() {
  while(RawHID.available()) {
    byte incomingBuffer[64];
    int responseLength = RawHID.recv(incomingBuffer, 0);
    if (responseLength > 0) {
      if (usbPosition + responseLength >= WIDTH * VHEIGHT) {
        responseLength = WIDTH * VHEIGHT - usbPosition;
      }
      memcpy(screenBuffer + usbPosition, incomingBuffer, responseLength);
      usbPosition += responseLength;
      if (usbPosition >= WIDTH * VHEIGHT) {
        usbPosition = 0;
      }
    }
  }
}

void drawBuffer(int x, int y) {
  // x value starts at sample 9, y value starts at line 19
  int i = x - 9;
  int j = VHEIGHT * (y - 19) / HEIGHT;
  uint8_t px = screenBuffer[i + j * WIDTH];
  writeComposite(max(1,px));
}

void scanline(int x, int y) {
  if (x < 5) {
    // sync
    writeComposite(0);
  }
  else if(x < 10) {
    // back porch
    writeComposite(1);
  }
  else if(x < 62) {
    drawBuffer(x, y);
  }
  else {
    writeComposite(1);
  }
}

void vsync(int x, int y) {
  if (y < 9) {
    // 9 pulse lines:
    // 6 pre equalizing pulses
    // 6 sync pulses
    // 6 post equalizing pulses
    int pulse = y/3 % 2;
    int pulseDelay = pulse ? 4 : 2;
    int hx = x / 2;
    if (hx < pulseDelay) {
      writeComposite(pulse ? 1 : 0);
    }
    else {
      writeComposite(pulse ? 0 : 1);
    }
  }
  else {
    // 11 black lines
    writeComposite(x < 5 ? 0 : 1);
  }
}

void halfline(int x) {
  writeComposite(x < 6 ? 1 : 0);
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
    int x = ts % 63;
    int y = ts / 63;
    if(ts == 0) {
      readRawHID();
    }
    if (y < 20) {
      vsync(x, y);
    }
    else if(y < 263) {
      scanline(x, y);
    }
    else if(ts < 16538) { // admittedly hand-tuned, should be 16537-16667
      halfline(x);
    }
    else {
      ts = 0;
      frameCount++;
    }
  }
}
