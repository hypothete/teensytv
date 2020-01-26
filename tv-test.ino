#define VID_PIN 15
#define SYNC_PIN 14

#define WIDTH 52
#define HEIGHT 242
#define VHEIGHT 39

int lineCount = 0;
bool evenFrame = true;
int frameCount = 0;
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
    dst_buf[i] = x % 2 ^ y % 2;
//    dst_buf[i] = hypo < H2 * (sin((float)(frameCount / 32.0)) + 1);
  }
}

void setPx(int x, int y, bool val) {
  dst_buf[x + WIDTH * y] = val;
}

void drawBuffer() {
  for(int i=0; i < WIDTH; i++) {
    int y = VHEIGHT * lineCount / HEIGHT;
    digitalWriteFast(VID_PIN, dst_buf[i + y * WIDTH]);
    delayMicroseconds(1);
  }
}

void scanline() {
  // sync
  digitalWriteFast(VID_PIN, LOW);
  digitalWriteFast(SYNC_PIN,LOW);
  delayMicroseconds(5);

  // back porch
  digitalWriteFast(VID_PIN,  LOW);
  digitalWriteFast(SYNC_PIN,HIGH);
  delayMicroseconds(4);

  // picture
  drawBuffer();

  // front porch
  digitalWriteFast(VID_PIN,  LOW);
  digitalWriteFast(SYNC_PIN,HIGH);
  delayMicroseconds(1);
}

void vsync() {
  digitalWriteFast(VID_PIN,  LOW);
  // 9 pulse frames
  // 6 pre equalizing pulses
  for(int i=0; i<5; i++) {
    digitalWriteFast(SYNC_PIN,LOW);
    delayMicroseconds(5);
    digitalWriteFast(SYNC_PIN,HIGH);
    delayMicroseconds(27);
  }
  // 6 vertical sync pulses
  for(int i=0; i<5; i++) {
    digitalWriteFast(SYNC_PIN,HIGH);
    delayMicroseconds(2);
    digitalWriteFast(SYNC_PIN,LOW);
    delayMicroseconds(30);
  }
  // 6 post equalizing pulses
  for(int i=0; i<5; i++) {
    digitalWriteFast(SYNC_PIN,LOW);
    delayMicroseconds(2);
    digitalWriteFast(SYNC_PIN,HIGH);
    delayMicroseconds(30);
  }
  // 11 black frames
  for(int i=0; i<10; i++) {
    digitalWriteFast(SYNC_PIN,LOW);
    delayMicroseconds(5);
    digitalWriteFast(SYNC_PIN,HIGH);
    delayMicroseconds(58);
  }
}

void halfline() {
  // just treating as filler for now
  digitalWriteFast(VID_PIN, LOW);
  digitalWriteFast(SYNC_PIN,LOW);
  delayMicroseconds(5);
  digitalWriteFast(SYNC_PIN,HIGH);
  delayMicroseconds(2);
}

void setup() {
  pinMode(VID_PIN,  OUTPUT);
  pinMode(SYNC_PIN, OUTPUT);
  setupBuffer();
}

void loop() {
  while(1) {
    vsync();
    for(lineCount=0; lineCount < HEIGHT; lineCount++) {
      scanline();
    }
    halfline();
    evenFrame = !evenFrame;
    frameCount++;
    setupBuffer();
  }
}
