#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int BTN_LEFT = 4;
const int BTN_RIGHT = 5;
const int BTN_ROTATE = 6;

// dimensiune ecran 64x128 - vom folosi 60x120
const int BLOCK_SIZE = 6; // 6 pixeli per patratel (10x6 = 60px latime)
const int BOARD_W = 10;
const int BOARD_H = 20;   // 20x6 = 120px inaltime (incape perfect in 128)

int board[BOARD_H][BOARD_W] = {0};

// Variabile stare
long score = 0;
bool gameOver = false;
int currentX, currentY, currentType;
unsigned long lastMoveTime = 0;
int gameSpeed = 300;

// Formele pieselor (7 tipuri)
const byte shapes[7][4][4] = {
  {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, // I
  {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}}, // O
  {{0,0,0,0},{0,1,0,0},{1,1,1,0},{0,0,0,0}}, // T
  {{0,0,0,0},{0,0,1,0},{1,1,1,0},{0,0,0,0}}, // L
  {{0,0,0,0},{1,0,0,0},{1,1,1,0},{0,0,0,0}}, // J
  {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}}, // S
  {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}}  // Z
};
byte piece[4][4];

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_ROTATE, INPUT_PULLUP);
  
  // Initializare OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;); 
    //daca nu s-a reusit initializarea se blocheaza programul aici
  }

  // 1 = 90 grade (Portret). Acum coordonatele X merg pana la 64, Y pana la 128
  display.setRotation(1); 
  
  resetGame();
}

void loop() {
  // daca jocul este gata si se apasa pe butonul de rotire atunci se reincepe jocul
  if (gameOver) {
    showGameOver();
    if (digitalRead(BTN_ROTATE) == LOW) {
      resetGame();
      delay(500);
    }
    return;
  }

  // se citeste inputul de la butoane
  if (digitalRead(BTN_LEFT) == LOW) { movePiece(-1, 0); delay(100); }
  if (digitalRead(BTN_RIGHT) == LOW) { movePiece(1, 0); delay(100); }
  if (digitalRead(BTN_ROTATE) == LOW) { rotatePiece(); delay(200); }

  // piesa curenta coboara
  if (millis() - lastMoveTime > gameSpeed) {
    if (!movePiece(0, 1)) {
      // piesa a lovit jos
      lockPiece();
      checkLines();
      newPiece();
      
      // verificam daca noua piesa are loc iar in caz negativ semnalam ca jocul se termina
      if (!isValid(0, 0)) {
         gameOver = true;
      }
    }
    lastMoveTime = millis();
  }

  // desenam tabla de joc
  drawGame();
}


// curatam tabla
void resetGame() {
  memset(board, 0, sizeof(board));
  score = 0;
  gameOver = false;
  gameSpeed = 300;
  newPiece();
}

// cream piesa noua
void newPiece() {
  currentType = random(0, 7); // alegem random o piesa din vector
  currentX = 3; 
  currentY = 0;

  // copiem piesa
  for(int y=0; y<4; y++) 
    for(int x=0; x<4; x++) 
      piece[y][x] = shapes[currentType][y][x];
}


// verificam limitele si coliziunile pentru a vedea ca urmatoarea mutare este valida
bool isValid(int offsetX, int offsetY) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (piece[y][x]) {
        int newX = currentX + x + offsetX;
        int newY = currentY + y + offsetY;
        if (newX < 0 || newX >= BOARD_W || newY >= BOARD_H || (newY >= 0 && board[newY][newX])) 
          return false;
      }
    }
  }
  return true;
}

// mutarea piesei
bool movePiece(int dx, int dy) {
  if (isValid(dx, dy)) {
    currentX += dx;
    currentY += dy;
    return true;
  }
  return false;
}

void rotatePiece() {
  byte temp[4][4];
  // rotire matrice 4x4
  for(int y=0; y<4; y++) for(int x=0; x<4; x++) temp[x][3-y] = piece[y][x];
  
  byte backup[4][4];
  memcpy(backup, piece, 16);
  memcpy(piece, temp, 16);
  
  if (!isValid(0, 0)) memcpy(piece, backup, 16); // revert daca nu e loc
}

//cand s-a lovit de baza o imprimam in tabla de joc
void lockPiece() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (piece[y][x]) board[currentY + y][currentX + x] = 1;
    }
  }
}

void checkLines() {
  for (int y = BOARD_H - 1; y >= 0; y--) {
    bool full = true;
    for (int x = 0; x < BOARD_W; x++) if (!board[y][x]) full = false;
    
    if (full) {
      // linie completa
      score += 100;
      if(gameSpeed > 50) gameSpeed -= 5;//viteza jocului creste putin cate putin
      
      for (int i = y; i > 0; i--) 
        for (int x = 0; x < BOARD_W; x++) board[i][x] = board[i-1][x];
      y++; 
    }
  }
}

// --- DESENARE ---
void drawGame() {
  display.clearDisplay();
  
  // Desenare cadru (Offset 1 pixel ca sa arate bine)
  int offsetX = 2; 
  int offsetY = 2;

  // bordura
  display.drawRect(0, 0, BOARD_W * BLOCK_SIZE + 4, BOARD_H * BLOCK_SIZE + 4, SSD1306_WHITE);

  // piese pe tabla
  for (int y = 0; y < BOARD_H; y++) {
    for (int x = 0; x < BOARD_W; x++) {
      if (board[y][x]) 
        display.fillRect(offsetX + x * BLOCK_SIZE, offsetY + y * BLOCK_SIZE, BLOCK_SIZE-1, BLOCK_SIZE-1, SSD1306_WHITE);
    }
  }
  
  // piesa curenta
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (piece[y][x])
         display.fillRect(offsetX + (currentX + x) * BLOCK_SIZE, offsetY + (currentY + y) * BLOCK_SIZE, BLOCK_SIZE-1, BLOCK_SIZE-1, SSD1306_WHITE);
    }
  }

  // afisare scor mic sus 
  display.setCursor(BOARD_W * BLOCK_SIZE + 6, 0);
  display.print(score);
  
  display.display();
}

void showGameOver() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(5, 30);
  display.println(F("GAME OVER"));
  
  display.setCursor(5, 40);
  display.print(F("Scor: "));
  display.println(score);
  
  display.setCursor(17, 80);
  display.println(F("Apasa"));
  display.setCursor(1, 90);
  display.println(F("BTN ROTIRE"));

  display.setCursor(3, 110);
  display.println(F("pt restart"));
  
  display.display();
}