#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- PINI ---
const int BTN_LEFT = 4;
const int BTN_RIGHT = 5;
const int BTN_ROTATE = 6;



// --- STATE MACHINE ---
enum SystemState { STATE_MENU, STATE_PLAYING, STATE_GAMEOVER };
SystemState currentState = STATE_MENU;

bool selectedGameIsTetris = true; // true = Tetris, false = NFS
bool scoreSent = false;



// dimensiune ecran 64x128 - vom folosi 60x120
const int BLOCK_SIZE = 6; // 6 pixeli per patratel (10x6 = 60px latime)
const int BOARD_W = 10;
const int BOARD_H = 20;   // 20x6 = 120px inaltime (incape perfect in 128)

int board[BOARD_H][BOARD_W] = {0};



// Variabile stare
long score = 0;
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

const byte car[4][3] = {
  {0,1,0},{1,1,1},{0,1,0},{1,1,1}
};

int mapOffset = 0;
const byte NFSmap[30][10] = {
  {1,1,0,0,0,0,0,0,1,1}, {1,1,1,0,0,0,0,1,1,1}, {1,1,1,0,0,0,0,1,1,1},
  {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1},
  {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1}, {1,1,1,0,0,0,0,1,1,1},
  {1,1,1,0,0,0,0,0,1,1}, {1,1,1,0,0,0,0,0,1,1}, {1,1,1,0,0,0,0,0,1,1},
  {1,1,1,0,0,0,0,0,1,1}, {1,1,1,0,0,0,0,0,1,1}, {1,1,1,0,0,0,0,0,1,1},
  {1,1,1,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1},
  {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1}, {1,1,1,0,0,0,0,0,1,1},
  {1,1,1,1,0,0,0,0,1,1}, {1,1,1,1,0,0,0,0,1,1}, {1,1,1,1,0,0,0,0,1,1},
  {1,1,1,1,0,0,0,0,1,1}, {1,1,1,1,0,0,0,0,1,1}, {1,1,1,0,0,0,0,1,1,1},
  {1,1,1,0,0,0,0,1,1,1}, {1,1,1,0,0,0,0,1,1,1}, {1,1,0,0,0,0,0,0,1,1}
};




// --- SETUP ---
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
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
}



// --- LOOP + MENIU ---
void loop() {
  switch (currentState) {
    case STATE_MENU:
      runMenuLogic();
      break;
      
    case STATE_PLAYING:
      if (selectedGameIsTetris) {
        runTetrisGame();
      } else {
        runNFSGame();
      }
      break;
      
    case STATE_GAMEOVER:
      showGameOver();
      break;
  }
}

void runMenuLogic() {
  // initializare meniu
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // titlu
  display.setCursor(5, 10);
  display.println(F("ALEGE JOC:"));
  display.drawLine(0, 20, 64, 20, SSD1306_WHITE);

  // optiunea 1: TETRIS
  if (selectedGameIsTetris) {
    display.fillRect(0, 40, 64, 14, SSD1306_WHITE); // Highlight
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(5, 43);
    display.println(F("> TETRIS"));
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 43);
    display.println(F("  TETRIS"));
  }

  // optiunea 2: NFS
  if (!selectedGameIsTetris) {
    display.fillRect(0, 60, 64, 14, SSD1306_WHITE); // Highlight
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(5, 63);
    display.println(F("> NFS"));
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 63);
    display.println(F("  NFS"));
  }

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 90);
  display.println(F("Stg/Dr:"));
  display.println(F("Alege"));
  display.println(F("Rotire:"));
  display.println(F("Start"));

  display.display();

  // input Meniu
  if (digitalRead(BTN_LEFT) == LOW) {
    selectedGameIsTetris = true;
    delay(150);
  }
  if (digitalRead(BTN_RIGHT) == LOW) {
    selectedGameIsTetris = false;
    delay(150);
  }
  if (digitalRead(BTN_ROTATE) == LOW) {
    if (selectedGameIsTetris) resetTetris();
      else resetNFS();
    
    currentState = STATE_PLAYING;
    delay(300);
  }
}



// --- FUNCTII NFS ---
bool wallColision(int offsetX, int offsetY) {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 3; x++) {
      if (car[y][x]) {
        int screenX = x + offsetX;
        int screenY = y + offsetY;
        
        if (screenX < 0 || screenX >= 10)
          return true;

        int mapY = (screenY + 30 - mapOffset) % 30;
        if (NFSmap[mapY][screenX] == 1)
        return true;
      }
    }
  }
  return false;
}

void runNFSGame(){
  if (digitalRead(BTN_LEFT) == LOW) { 
    if (currentX > 2) 
      currentX--; 
    delay(100); 
  }
  if (digitalRead(BTN_RIGHT) == LOW) { 
    if (currentX < 5) 
      currentX++; 
    delay(100); 
  }


  if (digitalRead(BTN_ROTATE) == LOW)
    gameSpeed = 100;
  else
    gameSpeed = 300;
  

  if (millis() - lastMoveTime > gameSpeed) {
    score+=10;
    mapOffset++;
    if (mapOffset >= 30)
      mapOffset = 0;
    // verificam daca nu am facut accident
    if (wallColision(currentX, currentY) == true) {
          currentState = STATE_GAMEOVER; 
       }
    lastMoveTime = millis();
  }

  // desenam tabla de joc
  drawNFS();
}

// curatam tabla
void resetNFS() {
  memset(board, 0, sizeof(board));
  score = 0;
  gameSpeed = 300;
  mapOffset = 0;

  currentX = 3;
  currentY = 15;
}

void drawNFS() {
  display.clearDisplay();
  
  // Desenare cadru (Offset 1 pixel ca sa arate bine)
  int offsetX = 2; 
  int offsetY = 2;

  // bordura
  display.drawRect(0, 0, BOARD_W * BLOCK_SIZE + 4, BOARD_H * BLOCK_SIZE + 4, SSD1306_WHITE);
  
  //harta
  for (int y = 0; y < BOARD_H; y++) {
    int mapY = (y + 30 - mapOffset) % 30;
    
    for (int x = 0; x < 10; x++) {
      if (NFSmap[mapY][x] == 1) {
         display.fillRect(offsetX + x * BLOCK_SIZE, offsetY + y * BLOCK_SIZE, BLOCK_SIZE-1, BLOCK_SIZE-1, SSD1306_WHITE);
      }
    }
  }

  // masina
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 3; x++) {
      if (car[y][x])
         display.fillRect(offsetX + (currentX + x) * BLOCK_SIZE, offsetY + (currentY + y) * BLOCK_SIZE, BLOCK_SIZE-1, BLOCK_SIZE-1, SSD1306_WHITE);
    }
  }
  display.display();
}



// --- FUNCTII TETRIS ---
void runTetrisGame() {

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
         currentState = STATE_GAMEOVER;
      }
    }
    lastMoveTime = millis();
  }

  // desenam tabla de joc
  drawTetris();
}

// curatam tabla
void resetTetris() {
  memset(board, 0, sizeof(board));
  score = 0;
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
void drawTetris() {
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



// --- AFISARE SFARSIT JOC ---
void trimiteScorLaESP(String numeJoc, long scorFinal) {
  // Trimitem in formatul: "NUME:SCOR"
  Serial1.print(numeJoc);
  Serial1.print(":");
  Serial1.println(scorFinal);

  Serial.print(numeJoc);
  Serial.print(":");
  Serial.println(scorFinal);
}

void showGameOver() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(10, 30);
  display.println(F("GAME OVER"));
  
  display.setCursor(10, 50);
  display.print(F("Scor: "));
  display.println(score);
  
  display.setCursor(17, 90);
  display.println(F("Apasa"));
  display.setCursor(17, 100);
  display.println(F("ROTIRE"));
  display.setCursor(10, 110);
  display.println(F("ptr MENIU"));
  
  display.display();

  // Trimitem o singura data cand intram in ecranul de Game Over
  if (!scoreSent) {
    if (selectedGameIsTetris) {
      trimiteScorLaESP("TETRIS", score);
    } else {
      trimiteScorLaESP("NFS", score);
    }
    scoreSent = true; 
  }

  if (digitalRead(BTN_ROTATE) == LOW) {
    currentState = STATE_MENU;
    scoreSent = false;
    delay(500);
  }
}