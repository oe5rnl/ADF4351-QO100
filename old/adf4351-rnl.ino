// Den ADF4351 Frequenzgenerator ansteuern vom Arduino Uno
// Ausgang 35 MHz bis 4,4 GHz
//
// Spannungsteiler Uno 5V > ADF4351 Daten 3V:
// MOSI (pin 11) über Spannungswandler auf ADF DATA
// SCK (pin13) über Spannungswandler auf ADF CLK
// Select (PIN 3) über Spannungswandler auf ADF LE
//
// Direkt AFD4351 Daten > Uno
// ADF MUXOUT direkt an Arduino Pin 2 für Lock detect
// http://shelvin.de/den-adf4351-frequenzgenerator-35mhz-bis-44ghz-vom-arduino-uno-ansteuern/
//
// von Matthias Busse 10.3.2018 Version 1.0 auf shelvin.de

// 20190426: OE5RNL: 25MHz740NHz Oszillaoor for Eshaislsat QO100

#include <SPI.h>
#define LE 3 // Load Enable Pin 3

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

void setup() {
  Serial.begin (38400); 
  Serial.println("1");
  pinMode(lockdetect, INPUT);     // MUX Eingang für lock detect
  pinMode(LE, OUTPUT);            // Load Enable Pin
  digitalWrite(LE, HIGH);
  SPI.begin();                    // Init SPI bus
  SPI.setDataMode(SPI_MODE0);     // CPHA = 0  Clock positive
  SPI.setBitOrder(MSBFIRST);           
  
  SetFrequencyADF4351(40.0);     //437 MHz Startwert
  SetPowerADF4351(5);
  
  //Serial.println("2");
  //pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("3");
}

//void loop() {
//
//Serial.println("oe5rnl");
//}
  int x;
 
void loop() {
  int ser, len, p;
  char buf[10];
  double freq;

 Serial.println(x);
  x = x+1;
//  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(500);                       // wait for a second
//  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);  

//  Serial.println("m2");

//   Serial.println("adf4351-oe5rnl");
//  if (Serial.available() > 0) {
//    ser = Serial.read();
//     Serial.println(ser);
//    if(ser == 'h') {                          // Hilfe Text ausgeben
//      Serial.println("Help");
//      Serial.println(" f: set frequency 34...4400 MHz. Examples: f1000 | f300.35 | f 2000 | f99.99 ");
//      Serial.println(" g: get frequency");
//      Serial.println(" p: set output power (dBm). Allowed values are: p-4 | p-1 | p2 | p5");
//      Serial.println(" o: get output Power");
//      Serial.println(" d: device class");
//      Serial.println(" w: who");
//      Serial.println(" v: version");
//      Serial.println(" 1: min. frequency");
//      Serial.println(" 2: max. frequency");
//      Serial.println(" 3: min. power");
//      Serial.println(" 4: max. power");
//      Serial.println(" 5: power step width");
//    }  
//    if(ser == 'd') Serial.println("Sythesizer"); // device ?
//    if(ser == 'w') Serial.println("ADF4351"); // who ?
//    if(ser == 'v') Serial.println("1.0");     // version ?
//    if(ser == '1') Serial.println("35000000"); // 35 MHz fmin
//    if(ser == '2') Serial.println("4400000000"); // 4.4 GHz fmax
//    if(ser == '3') Serial.println("-4");      // -4  dB Pout min.
//    if(ser == '4') Serial.println("5");       // +5 dB Pout max.
//    if(ser == '5') Serial.println("3");       // Pout step
//    if(ser == 'o') Serial.println(RFpower);   // Ausgangsleistung in dBm
//    if(ser == 'g') Serial.println(RFout);     // Frequenz in MHz
//    if(ser == 'f') {                          // Frequenz einstellen: f437 | f 1000
//      Serial.setTimeout(15);
//      len = Serial.readBytes(buf,12);
//      if((len>2) && (len<11)) {               // f und was dahinter ?
//        freq = atof (buf);
//        if((freq >= 34.0) && (freq <= 4400.0)) {
//          SetFrequencyADF4351(freq);
//        }
//      }
//    }
//    if(ser == 'p') {                          // Ausgangsleistung einstellen : p-4 | p-1 | p2 | p5
//      Serial.setTimeout(15);
//      len = Serial.readBytes(buf,8);
//      //Serial.println(len);
//      if((len>2) && (len<6)) {                // p und was dahinter ?
//        p = atoi (buf);
//        if(p==-4) {RFpower=-4; SetPowerADF4351(RFpower);}
//        if(p==-1) {RFpower=-1; SetPowerADF4351(RFpower);}
//        if(p==2) {RFpower=2; SetPowerADF4351(RFpower);}
//        if(p==5) {RFpower=5; SetPowerADF4351(RFpower);}
//      }  
//    }
//    Serial.setTimeout(500);  
//  }
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

//--------------------------------------------
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
  // ???????????? !!!!!!!!!!!!!!! while(digitalRead(lockdetect)==0) {}        // warten bis lock detect erkannt wird
}

//--------------------------------------------
void WriteRegADF() {
  for (int i = 5; i >= 0; i--){               // die 6 Register schreiben
    WriteRegister32(registers[i]);
  }
}

//--------------------------------------------
void WriteRegister32(const uint32_t value) {  // 32 Bit Register schreiben
  digitalWrite(LE, LOW);
  for (int i = 3; i >= 0; i--) {        
    SPI.transfer((value >> 8 * i) & 0xFF);
  } 
  digitalWrite(LE, HIGH);
  digitalWrite(LE, LOW);
}
