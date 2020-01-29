#define WIDTH 52
#define HEIGHT 242
#define VHEIGHT 39

volatile int frameCount = 0;
volatile bool vblank = false;
uint8_t * dst_buf = (uint8_t*)malloc(WIDTH * VHEIGHT);

void writeComposite(uint8_t v) {
  int fv = 0xf & v;
  int a = 0x1 & fv;
  int b = 0x2 & fv;
  int c = 0x4 & fv;
  int d = 0x8 & fv;
  digitalWriteFast(14, a);
  digitalWriteFast(15, b);
  digitalWriteFast(16, c);
  digitalWriteFast(17, d);
}

void setupBuffer() {
//  int H2 = VHEIGHT / 2;
//  int W2 = WIDTH / 2;
  for(int i=0; i< WIDTH * VHEIGHT; i++) {
    int x = i % WIDTH;
    int y = i / WIDTH;
//    int dx = W2 - x;
//    int dy = H2 - y;
//    int hypo = sqrt(dx*dx + dy*dy);
    dst_buf[i] = 9 + (float)( 3.0 * (sin(frameCount / 13.0 + y / 4.0) - cos(frameCount / 17.0 + x / 3.0)));
  }
}

void drawBuffer(int x, int y) {
  // x value starts at sample 9, y value starts at line 19
  int i = x - 9;
  int j = VHEIGHT * (y - 19) / HEIGHT;
  uint8_t px = dst_buf[i + j * WIDTH];
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
  pinMode(14,  OUTPUT);
  pinMode(15,  OUTPUT);
  pinMode(16,  OUTPUT);
  pinMode(17,  OUTPUT);
  setupBuffer();
}

void loop() {
  elapsedMicros ts;
  while(1) {
    int x = ts % 63;
    int y = ts / 63;
    if (ts==0) {
      setupBuffer();
    }
    if (y < 20) {
      vsync(x, y);
    }
    else if(y < 263) {
      scanline(x, y);
    }
    else if(ts < 16637) { // admittedly hand-tuned, should be 16537-16667
      halfline(x);
    }
    else {
      ts = 0;
      frameCount++;
    }
  }
}
