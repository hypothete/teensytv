#define VID_PIN 15
#define SYNC_PIN 14

#define WIDTH 52
#define HEIGHT 242
#define VHEIGHT 39

volatile int frameCount = 0;
volatile bool vblank = false;
bool * dst_buf = (bool*)malloc(WIDTH * VHEIGHT);

void setupBuffer() {
  for(int i=0; i< WIDTH * VHEIGHT; i++) {
    int x = i % WIDTH;
    int y = i / WIDTH;
    int H2 = VHEIGHT / 2;
    int W2 = WIDTH / 2;
    int dx = W2 - x;
    int dy = H2 - y;
    int hypo = sqrt(dx*dx + dy*dy);
    dst_buf[i] = sin((float)(hypo - frameCount / 2.0)) > 0;
  }
}

void drawBuffer(int x, int y) {
  int i = x - 9;
  int j = VHEIGHT * (y - 19) / HEIGHT;
  digitalWriteFast(VID_PIN, dst_buf[i + j * WIDTH]);
}

void scanline(int x, int y) {
  if (x < 5) {
    // sync
    digitalWriteFast(VID_PIN, LOW);
    digitalWriteFast(SYNC_PIN,LOW);
  }
  else if(x < 10) {
    // back porch
    digitalWriteFast(VID_PIN, LOW);
    digitalWriteFast(SYNC_PIN,HIGH);
  }
  else if(x < 62) {
    drawBuffer(x, y);
//    digitalWriteFast(VID_PIN, HIGH);
    digitalWriteFast(SYNC_PIN,HIGH);
  }
  else {
    digitalWriteFast(VID_PIN,  LOW);
    digitalWriteFast(SYNC_PIN,HIGH);
  }
}

void vsync(int x, int y) {
  digitalWriteFast(VID_PIN,  LOW);
  if (y < 9) {
    // 9 pulse lines:
    // 6 pre equalizing pulses
    // 6 sync pulses
    // 6 post equalizing pulses
    int pulse = y/3 % 2;
    int pulseDelay = pulse ? 4 : 2;
    int hx = x / 2;
    if (hx < pulseDelay) {
      digitalWriteFast(SYNC_PIN,pulse);
    }
    else {
      digitalWriteFast(SYNC_PIN,!pulse);
    }
  }
  else {
    // 11 black lines
    if (x < 5) {
      digitalWriteFast(SYNC_PIN,LOW);
    }
    else { 
      digitalWriteFast(SYNC_PIN,HIGH);
    }
  }
}

void halfline(int x) {
  digitalWriteFast(VID_PIN, LOW);
  digitalWriteFast(SYNC_PIN, x < 6);
}

void setup() {
  pinMode(VID_PIN,  OUTPUT);
  pinMode(SYNC_PIN, OUTPUT);
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
