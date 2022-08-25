// Den ADF4351 Frequenzgenerator ansteuern vom Arduino Uno
// Ausgang 35 MHz bis 4,4 GHz
//
// Original von Matthias Busse 10.3.2018 Version 1.0 auf shelvin.de
//
// http://shelvin.de/den-adf4351-frequenzgenerator-35mhz-bis-44ghz-vom-arduino-uno-ansteuern/
//
// Der Quellenhinweis darf nicht entfernt werden !!! 
//
//
// 20190620: Geändert mit freundlicher Genehmigung von Matthias Busse.
//           von oe5rnl@oevsv.at für die Synchronistation eines Pluto SDR mit 40 MHz, 25 MHZ Referenz
//
// 20220823: Umstellung auf Microsoft Code mit platformio. Arduino IDE ist ebenfalls installiert.
//
// Verwendung eines Arduino 328 Pro Mini 3,3V !!!
// Das ADF4351 Board benötigt 3,3V Signale
//
// Die Ausgangsfrequenz wird nach dem Start fix auf 40 MHz eingestellt.
// Die Referenzfrequenz auf 25 MHz.
// Es besteht für Tests jedoch weiterhin die Möglichkeit die Frequenz und Amplitude 
// über die serielle Schnittstelle einzustellen.
//
// MOSI   (pin 11) auf ADF DATA
// SCK    (pin 13) auf ADF CLK
// Select (PIN 3)  auf ADF LE
// GPIO   (Pin 2)  auf ADF MUXOUT Lock detect


#include <SPI.h>
#include <avr/sleep.h>
#define LE 3 // Load Enable Pin 3

#define QRG 40.0
#define PWR 5

                                  // die 6 Register einfach für 500 MHz mit einem 25 MHz Quarz vorbelegen
uint32_t registers[6]={0x500000, 0x8011, 0x4E42, 0x04B3, 0xBC802C, 0x580005};
byte RFDivider;                   // Ausgangs-Frequenzteiler
unsigned int long MOD, FRAC, INT; // INT FRAC MOD Werte lt. Datenblatt 
unsigned int long quarzFreq=25;   // Quarzfrequenz
double FRACF;  
double RFout;                     // Ausgangsfrequenz 
int RFpower=5;                    // Ausgangsleistung
double RFoutSchrittweite=0.01;    // 10 kHz
int lockdetect=2;                 // MIX Pin für Lock Detect

void WriteRegister32(const uint32_t value) {  // 32 Bit Register schreiben
  digitalWrite(LE, LOW);
  for (int i = 3; i >= 0; i--) {        
    SPI.transfer((value >> 8 * i) & 0xFF);
  } 
  digitalWrite(LE, HIGH);
  digitalWrite(LE, LOW);
}


void WriteRegADF() {
  for (int i = 5; i >= 0; i--){               // die 6 Register schreiben
    WriteRegister32(registers[i]);
  }
}

void SetPowerADF4351(int power) {
// Die Ausgangsleistung einstellen. 
// Datenblatt Seite 17

  if( power == -4) {
    bitWrite (registers[4], 4, 0);
    bitWrite (registers[4], 3, 0);
    WriteRegADF();
  }
  if( power == -1) {
    bitWrite (registers[4], 4, 0);
    bitWrite (registers[4], 3, 1);
    WriteRegADF();
  }
  if( power == 2) {
    bitWrite (registers[4], 4, 1);
    bitWrite (registers[4], 3, 0);
    WriteRegADF();
  }
  if( power == 5) {
    bitWrite (registers[4], 4, 1);
    bitWrite (registers[4], 3, 1);
    WriteRegADF();
  }
}

void SetFrequencyADF4351(double FreqMHz) {
// Die Frequenz einstellen. 

  RFout = FreqMHz; // aktuelle Frequenz speichern
  if (FreqMHz >= 2200) { 
      RFDivider = 1;           // keín Teiler, das heisst Sinus Frequenz Ausgang 2,2 ... 4,4 GHz
      bitWrite (registers[4], 22, 0);
      bitWrite (registers[4], 21, 0);
      bitWrite (registers[4], 20, 0);
    }
  if (FreqMHz < 2200) {
    RFDivider = 2;            // Frequenzteiler, das heisst Rechteck Ausgang mit entsprechenden Oberwellen
    bitWrite (registers[4], 22, 0);
    bitWrite (registers[4], 21, 0);
    bitWrite (registers[4], 20, 1);
  }
  if (FreqMHz < 1100) {
    RFDivider = 4;
    bitWrite (registers[4], 22, 0);
    bitWrite (registers[4], 21, 1);
    bitWrite (registers[4], 20, 0);
  }
  if (FreqMHz < 550)  {
    RFDivider = 8;
    bitWrite (registers[4], 22, 0);
    bitWrite (registers[4], 21, 1);
    bitWrite (registers[4], 20, 1);
  }
  if (FreqMHz < 275)  {
    RFDivider = 16;
    bitWrite (registers[4], 22, 1);
    bitWrite (registers[4], 21, 0);
    bitWrite (registers[4], 20, 0);
  }
  if (FreqMHz < 137.5) {
    RFDivider = 32;
    bitWrite (registers[4], 22, 1);
    bitWrite (registers[4], 21, 0);
    bitWrite (registers[4], 20, 1);
  }
  if (FreqMHz < 68.75) {
    RFDivider = 64;
    bitWrite (registers[4], 22, 1);
    bitWrite (registers[4], 21, 1);
    bitWrite (registers[4], 20, 0);
  }
  INT = (FreqMHz * RFDivider) / quarzFreq;    // PLL Teiler
  MOD = (quarzFreq / RFoutSchrittweite);
  FRACF = (((FreqMHz * RFDivider) / quarzFreq) - INT) * MOD;
  FRAC = round(FRACF); 
  registers[0] = 0;                           // Datenblatt Seite 15
  registers[0] += INT << 15;
  registers[0] += FRAC << 3;
  registers[1] = 1; 
  registers[1] += MOD << 3;
  bitSet (registers[1], 27);                  // Prescaler 8/9
  bitSet (registers[2], 28);                  // Datenblatt Seite 16
  bitSet (registers[2], 27);                  // Digital Lock Detect ist 110 
  bitClear (registers[2], 26); 
  WriteRegADF();
  
  while(digitalRead(lockdetect)==0) {}        // warten bis lock detect 1 wird

}


void setup() {
  Serial.begin (38400); 

 // Serial.println("start");

  pinMode(lockdetect, INPUT);     // MUX Eingang für lock detect
  pinMode(LE, OUTPUT);            // Load Enable Pin
  digitalWrite(LE, HIGH);
  SPI.begin();                    // Init SPI bus
  SPI.setDataMode(SPI_MODE0);     // CPHA = 0  Clock positive
  SPI.setBitOrder(MSBFIRST);           

  delay(200);

  SetFrequencyADF4351(QRG);    
  SetPowerADF4351(PWR);
 
  delay(2000);
  SetFrequencyADF4351(QRG);    
  SetPowerADF4351(PWR);

  delay(3000);
  SetFrequencyADF4351(QRG);    
  SetPowerADF4351(PWR);
   
}

void loop() {

  if (digitalRead(lockdetect)==0) {
    delay(1000);
    SetFrequencyADF4351(QRG);    
    SetPowerADF4351(PWR);
  }
  delay(1000);

}
