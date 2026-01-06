#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

// --- CONFIGURARE WIFI ---
const char *SSID = "Proiect_Jocuri";
const char *PASS = "parola1234"; 
const int HTTP_PORT_NO = 80;

WiFiServer HttpServer(HTTP_PORT_NO);

// --- BAZA DE DATE (RAM) ---
const int MAX_SCORURI = 10;
int tetrisScores[MAX_SCORURI] = {0};
int nfsScores[MAX_SCORURI] = {0};
int tetrisCount = 0;
int nfsCount = 0;

// --- HTML & CSS ---
const String HTTP_HEADER = "HTTP/1.1 200 OK\r\nContent-type:text/html\r\n\r\n";
const String CSS_STYLE = R"(
<style>
  body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; text-align: center; background-color: #f0f2f5; color: #333; }
  h1 { color: #2c3e50; margin-top: 30px; }
  .container { display: flex; justify-content: center; gap: 40px; flex-wrap: wrap; margin-top: 20px; }
  .card { background: white; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); overflow: hidden; width: 300px; }
  .card-header { padding: 15px; font-size: 1.2em; font-weight: bold; color: white; }
  .tetris-head { background-color: #3498db; }
  .nfs-head { background-color: #e74c3c; }
  table { width: 100%; border-collapse: collapse; }
  th, td { padding: 10px; border-bottom: 1px solid #eee; text-align: center; }
  tr:last-child td { border-bottom: none; }
  tr:nth-child(even) { background-color: #f9f9f9; }
</style>
)";

// --- FUNCTIE ADAUGARE SCOR ---
void adaugaScor(String joc, int scor) {
  int *targetArray;
  int *count;

  if (joc == "TETRIS") {
    targetArray = tetrisScores;
    count = &tetrisCount;
  } else if (joc == "NFS") {
    targetArray = nfsScores;
    count = &nfsCount;
  } else {
    return; 
  }

  // adaugam scorul la final
  if (*count < MAX_SCORURI) {
    targetArray[*count] = scor;
    (*count)++;
  } else {
    targetArray[MAX_SCORURI - 1] = scor; // suprascriem ultimul daca e plin
  }
  
  // sortare Descrescatoare
  for(int i=0; i<(*count)-1; i++) {
    for(int j=i+1; j<(*count); j++) {
      if(targetArray[i] < targetArray[j]) {
        int temp = targetArray[i];
        targetArray[i] = targetArray[j];
        targetArray[j] = temp;
      }
    }
  }
}

// --- PARSARE DATE SERIALE ---
void processSerialData(String data) {
  data.trim(); // scoatem spatiile goale sau \n de la final
  
  int separatorIndex = data.indexOf(':');
  
  if (separatorIndex > 0) {
    // extragem numele jocului (de la 0 pana la :)
    String numeJoc = data.substring(0, separatorIndex);
    // extragem scorul (de dupa : pana la final)
    String scorString = data.substring(separatorIndex + 1);
    int scor = scorString.toInt();
    
    adaugaScor(numeJoc, scor);
  }
}

void setup() {
  Serial.begin(115200); 
  
  WiFi.softAP(SSID, PASS);
  HttpServer.begin();
  
}

void sendScorePage(WiFiClient &client) {
  client.print(HTTP_HEADER);
  client.println("<!DOCTYPE html><html><head>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  client.println(CSS_STYLE);
  client.println("</head><body>");

  client.println("<h1>Clasament Arcade</h1>");
  client.println("<div class='container'>");

  // Tabel TETRIS
  client.println("<div class='card'><div class='card-header tetris-head'>TETRIS</div>");
  client.println("<table><tr><th>Loc</th><th>Puncte</th></tr>");
  for (int i = 0; i < tetrisCount; i++) {
    client.print("<tr><td>"); client.print(i + 1); client.print("</td><td>");
    client.print(tetrisScores[i]); client.println("</td></tr>");
  }
  client.println("</table></div>");

  // Tabel NFS
  client.println("<div class='card'><div class='card-header nfs-head'>NFS</div>");
  client.println("<table><tr><th>Loc</th><th>Puncte</th></tr>");
  for (int i = 0; i < nfsCount; i++) {
    client.print("<tr><td>"); client.print(i + 1); client.print("</td><td>");
    client.print(nfsScores[i]); client.println("</td></tr>");
  }
  client.println("</table></div>");

  client.println("</div></body></html>");
}

void loop() {
  if (Serial.available() > 0) {
    String mesajPrimit = Serial.readStringUntil('\n');
    processSerialData(mesajPrimit);
  }

  // --- VERIFICARE WEB SERVER ---
  WiFiClient client = HttpServer.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            sendScorePage(client);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
  }
}