#define VID_PIN 15
#define SYNC_PIN 14
#define MIN_X 2
#define MIN_Y 8
#define MAX_X 50
#define MAX_Y 240
#define WIDTH 48
#define HEIGHT 29

int lineCount = 0;
bool evenFrame = true;
int frameCount = 0;
bool * dst_buf = (bool*)malloc(WIDTH*HEIGHT);

void setupBuffer() {
  for(int i=0; i<WIDTH*HEIGHT; i++) {
    int x = i % WIDTH;
    int y = i / WIDTH;
//    dst_buf[i] = (8.0 * cos(y) - 8.0 * sin((float)(x - frameCount / 8.0))) > 0;
    int H2 = HEIGHT / 2;
    int W2 = WIDTH / 2;
    int dx = W2 - x;
    int dy = H2 - y;
    int hypo = sqrt(dx*dx + dy*dy);
    dst_buf[i] = hypo < 16 * (sin((float)(frameCount / 32.0)) + 1);
  }
}

void setPx(int x, int y, bool val) {
  dst_buf[x + WIDTH * y] = val;
}

void drawBuffer() {
  for(int i=0; i<52; i+=2) {
    if (i >= MIN_X && i < MAX_X && lineCount >= MIN_Y && lineCount < MAX_Y) {
      int mx = i - MIN_X;
      int my = HEIGHT * (lineCount - MIN_Y) / (MAX_Y - MIN_Y);
//      analogWrite(VID_PIN, dst_buf[mx + my * WIDTH] ? 0 : 255);
      digitalWrite(VID_PIN, dst_buf[mx + my * WIDTH] ? HIGH : LOW);
    }
    else {
      digitalWrite(VID_PIN, LOW);
    }
    
    delayMicroseconds(2);
  }
}

void setup() {
  pinMode(VID_PIN,  OUTPUT);
  pinMode(SYNC_PIN, OUTPUT);
  setupBuffer();
  analogWriteFrequency(VID_PIN, 375000);
//  setPx(10, 10, false);
//  setPx(31, 1, false);
//  setPx(32, 2, false);
}

void scanline() {
  // sync
  digitalWrite(VID_PIN, LOW);
  digitalWrite(SYNC_PIN,LOW);
  delayMicroseconds(5);

  // back porch
  digitalWrite(VID_PIN,  LOW);
  digitalWrite(SYNC_PIN,HIGH);
  delayMicroseconds(4);

  // picture
  drawBuffer();

  // front porch
  digitalWrite(VID_PIN,  LOW);
  digitalWrite(SYNC_PIN,HIGH);
  delayMicroseconds(1);
}

void vsync() {
  // 9 pulse frames
  // 6 pre equalizing pulses
  for(int i=0; i<5; i++) {
    digitalWrite(VID_PIN, LOW);
    digitalWrite(SYNC_PIN,LOW);
    delayMicroseconds(5);
    digitalWrite(VID_PIN,  LOW);
    digitalWrite(SYNC_PIN,HIGH);
    delayMicroseconds(27);
  }
  // 6 vertical sync pulses
  for(int i=0; i<5; i++) {
    digitalWrite(VID_PIN,  LOW);
    digitalWrite(SYNC_PIN,HIGH);
    delayMicroseconds(2);
    digitalWrite(VID_PIN, LOW);
    digitalWrite(SYNC_PIN,LOW);
    delayMicroseconds(30);
  }
  // 6 post equalizing pulses
  for(int i=0; i<5; i++) {
    digitalWrite(VID_PIN, LOW);
    digitalWrite(SYNC_PIN,LOW);
    delayMicroseconds(2);
    digitalWrite(VID_PIN,  LOW);
    digitalWrite(SYNC_PIN,HIGH);
    delayMicroseconds(30);
  }
  // 11 black frames
  for(int i=0; i<10; i++) {
    digitalWrite(VID_PIN, LOW);
    digitalWrite(SYNC_PIN,LOW);
    delayMicroseconds(5);
    digitalWrite(VID_PIN,  LOW);
    digitalWrite(SYNC_PIN,HIGH);
    delayMicroseconds(58);
  }
}

void halfline() {
  // just treating as filler for now
  digitalWrite(VID_PIN, LOW);
  digitalWrite(SYNC_PIN,LOW);
  delayMicroseconds(5);
  digitalWrite(VID_PIN,  LOW);
  digitalWrite(SYNC_PIN,HIGH);
  delayMicroseconds(26);
}

void loop() {
  vsync();
  for(lineCount=0; lineCount<241; lineCount++) {
    scanline();
  }
  halfline();
  evenFrame = !evenFrame;
  frameCount++;
  setupBuffer();
}
