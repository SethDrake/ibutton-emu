#include <OneWire.h>

#define pin 3
OneWire ibutton (pin); // Пин D11 для подлючения iButton (Data)
byte addr[8];
byte ReadID[8] = { 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F };
byte codeAfterWrite[8] = { 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F };

const int buttonPin = 5;
const int redLedPin = 8;
const int greenLedPin = 13;
int buttonState = 0;
int writeflag = 0;
int val = 0;
int codeReaded = 0;
int ibuttonIsPresent = 0;

void setup() {
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  
  for(byte i = 0; i < 2; i++) {
    digitalWrite(greenLedPin, HIGH);
    if (i == 1) {
      digitalWrite(redLedPin, HIGH);      
    }
    delay(400);
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, LOW); 
    delay(300);
  }
}

void loop() { 
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    if (codeReaded) {
      writeflag = 1;
    }
    val++;
    if (val > 3) val = 3;
    delay(300);
  }
  else {
    val = 0;
  }

  if (!ibutton.search(addr)) {
    ibutton.reset_search();
    delay(50);
    if ((val < 3) && !codeReaded) return;
    val = 0;
    ibuttonIsPresent = 0;
  } else {
    ibuttonIsPresent = 1;
  }
  
  delay(50);
  if ((ibuttonIsPresent == 1) && (codeReaded != 1)) {
    readCode(ReadID);
  }

  if (codeReaded) {
    digitalWrite(greenLedPin, HIGH);   
  } else {
    digitalWrite(greenLedPin, LOW);   
  }  

  if (writeflag)  {
     digitalWrite(redLedPin, HIGH);
     delay(150);
     digitalWrite(redLedPin, LOW);
     delay(150);
  }
  
  if (((codeReaded == 1) && (ibuttonIsPresent == 1) && (writeflag == 1)) or (Serial.read() == 'w')) {
    digitalWrite(redLedPin, HIGH);
    writeCode();
    delay(500);
    digitalWrite(redLedPin, LOW);
    Serial.print("Write performed.\n    ");
    ibutton.skip(); 
    ibutton.reset(); 
    Serial.print("Check code after write.\n    ");
    ibutton.search(addr);
    readCode(codeAfterWrite);
    if (codesAreEquals(ReadID, codeAfterWrite)) {
      Serial.print("Write successfull!\n    ");
      writeflag = 0;
      for(byte i = 0; i < 5; i++) {
        digitalWrite(greenLedPin, LOW);
        digitalWrite(redLedPin, HIGH);
        delay(150);
        digitalWrite(greenLedPin, HIGH);
        digitalWrite(redLedPin, LOW);
        delay(150);  
      }
    } else {
      ibutton.reset_search();
      Serial.print("Write error!\n    ");
    }
  }
}

void readCode(byte* code) {
  ibutton.skip(); 
  ibutton.reset(); 
  ibutton.write(0x33);
  for (byte x = 0; x < 8; x++) {
    code[x] = ibutton.read();
    Serial.print(code[x], HEX);
    Serial.print(":");
  }
  
  // CRC check
  byte crc = ibutton.crc8(code, 7);
  Serial.print("CRC: ");
  Serial.println(crc, HEX);
  if (code[7] == crc) {
    codeReaded = 1; 
  }  
}

void writeCode() {
  ibutton.skip(); 
  ibutton.reset(); 
  ibutton.write(0x33);
  Serial.print("ID before write:");
  for (byte x = 0; x < 8; x++) {
    Serial.print(' ');
    Serial.print(ibutton.read(), HEX);
  }
  // send reset
  ibutton.skip();
  ibutton.reset();
  // send 0xD1
  ibutton.write(0xD1);
  // send logical 0
  digitalWrite(pin, LOW); pinMode(pin, OUTPUT); delayMicroseconds(60);
  pinMode(pin, INPUT); digitalWrite(pin, HIGH); delay(10);

  Serial.print('\n');
  Serial.print("Writing iButton ID:\n    ");
  byte newID[8] = { (ReadID[0]), (ReadID[1]), (ReadID[2]), (ReadID[3]), (ReadID[4]), (ReadID[5]), (ReadID[6]), (ReadID[7]) };
  ibutton.skip();
  ibutton.reset();
  ibutton.write(0xD5);
  for (byte x = 0; x < 8; x++) {
    writeByte(newID[x]);
    Serial.print('*');
  }
  Serial.print('\n');
  ibutton.reset();
  // send 0xD1
  ibutton.write(0xD1);
  //send logical 1
  digitalWrite(pin, LOW); pinMode(pin, OUTPUT); delayMicroseconds(10);
  pinMode(pin, INPUT); digitalWrite(pin, HIGH); delay(10);  
}
 
int writeByte(byte data) {
  int data_bit;
  for (data_bit = 0; data_bit < 8; data_bit++) {
    if (data & 1) {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      delayMicroseconds(60);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    } else {
      digitalWrite(pin, LOW); pinMode(pin, OUTPUT);
      pinMode(pin, INPUT); digitalWrite(pin, HIGH);
      delay(10);
    }
    data = data >> 1;
  }
  return 0;
}

int codesAreEquals(byte a[], byte b[]) {
  for(byte i = 0; i < 8; i++) {
    if (a[i] != b[i]) {
      return 0;  
    }
  }
  return 1;
}
