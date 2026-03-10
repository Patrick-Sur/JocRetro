# Retro Brick Console: IoT-Enabled Portable Gaming

### Overview
**Retro Brick Console** este o consolă portabilă inspirată de clasicul "Brick Game", construită pe o arhitectură hibridă cu două microcontrolere (**Arduino Mega** și **ESP32**). Proiectul combină logica de joc retro cu tehnologii moderne, permițând monitorizarea scorurilor în timp real prin intermediul unei interfețe web.

### Key Features
* **Dual Game Engine:** Include două jocuri clasice implementate de la zero:
  * **Tetris:** Cu logică de rotație a pieselor (7 forme unice), detecție de coliziuni și sistem de gravitate.
  * **NFS (Racing):** Un joc de curse ce utilizează o matrice circulară pentru a genera o hartă infinită.
* **State Machine Architecture:** Navigare fluidă între meniuri și jocuri folosind o mașină de stări finită implementată în funcția `loop()`.
* **IoT Dashboard:** Transmiterea automată a scorurilor prin WiFi către o pagină web găzduită de ESP32.
* **Distributed Processing:** Arduino Mega gestionează gameplay-ul și randarea OLED, în timp ce ESP32 se ocupă de partea de comunicație și server web.

### Technical Stack
* **Microcontrollers:** Arduino Mega & ESP32.
* **Display:** Ecran OLED (comunicație I2C).
* **Communication:** Protocol **UART** pentru sincronizarea datelor între cele două unități de procesare.
* **Web:** HTML/CSS pentru dashboard-ul de scoruri, servit direct de pe ESP32.
* **Language:** C++ (utilizând concepte de manipulare a matricelor și gestiune a memoriei).

### Game Logic
1. **Tetris:**
   - **Harta:** Matrice `board[20][10]` pentru starea tablei.
   - **Piese:** Definite în matrice `4x4`, selectate aleatoriu la fiecare tură.
   - **Functii Cheie:** `isValid()` pentru coliziuni și `movePiece()` pentru translație și gravitate.
2. **NFS:**
   - **Infinite Scrolling:** Simulat prin variabila `mapOffset` (modulo 30), oferind iluzia unui traseu continuu.
   - **Coliziuni:** Funcția `wallColision()` verifică poziția mașinii față de obstacolele de pe hartă.

### Hardware Setup
* 1x Arduino Mega
* 1x ESP32
* 1x Ecran OLED (I2C)
* 3x Butoane tactile (Control direcție și acțiuni)

### Web Interface
La finalul fiecărui joc, scorul este serializat și trimis prin UART către ESP32. Acesta actualizează automat un tabel HTML accesibil în rețeaua locală, permițând vizualizarea performanțelor de pe orice smartphone sau laptop.
