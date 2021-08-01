#include <Adafruit_Fingerprint.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <LiquidCrystal.h>
//#include <SoftwareSerial.h>


// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
// uncomment this line:
// #define mySerial Serial1

// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// pin #7 is In from Relay
// comment these two lines if using hardware serial


//SoftwareSerial mySerial(2, 5);

LiquidCrystal lcd(46, 44, 40, 38, 36, 34);

int RECV_PIN = 12;
IRrecv irrecv(RECV_PIN); IRsend irsend;

decode_results results;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

uint8_t id;                   //ID del registro de huella
long previousWait = 0;        //Valores para el calculo del tiempo para apagar el registro de una nueva huella en caso de que no detecte por un tiempo
long waitOff = 5000;
bool primera = true;          //booleano para revisar si es la primera vez en el calculo del tiempo para el registro de huellas

const int EchoPin = 21;       //Pin echo del sensor Ultrasonico
const int TriggerPin = 25;    //Pin Trigger del sensor Ultrasonico
float distancia;              // Variable para el calculo de distancia del sensor ultrasonico
long tiempo;                  //Variable para el calculo de distancia del sensor ultrasonico

const int relay = 7;          //Pin del rele #1 (cerradura)
byte sensorpir = 10;          //pin del sensor PIR
byte relay2 = 4;              //Pin del rele #2 (foco)
byte relay3 = 11;             //Pin del rele #3 (foco)
bool automatic = true;        //variable para activar y desactivar el encendido de dispositivos automaticamente
int n = 0;                     // auto aire

int relayState = HIGH;        //Estado del/los rele (relevante para el calculo del tiempo en la funcion del temporizador (Sensor PIR))

unsigned long codigo;         //Variable para el codigo hexadecimal

long previousMillis = 0;        // will store last time LED was updated

long intervalOn = 50;           // ON
long intervalOff = 1800000;         // 30 min OFF

//SoftwareSerial BT1(8,9); // RX | TX
String W = " ";      //Variable relevante para la lectura de lineas en el monitor serial
char w ;            //variable relevante para la lectura de lineas en el monitor serial


//codigos RAW
unsigned int codigoAirePanaOn20[101] = {3450, 3400, 900, 2550, 900, 850, 850, 2550, 900, 850, 900, 800, 900, 2550, 900, 800, 900, 850, 850, 2550, 900, 850, 850, 2600, 850, 850, 900, 800, 900, 2550, 900, 850, 850, 850, 900, 800, 900, 2550, 900, 800, 900, 850, 850, 850, 850, 900, 850, 850, 900, 800, 900, 850, 850, 2550, 900, 850, 850, 850, 900, 850, 850, 850, 900, 800, 900, 850, 3450, 3400, 900, 2550, 900, 800, 900, 2550, 900, 800, 900, 850, 850, 2550, 900, 850, 900, 800, 900, 2550, 850, 850, 900, 2550, 900, 850, 850, 850, 850, 2550, 900, 850, 900, 800};
unsigned int codigoAirePanaOn[101] = {3550, 3350, 950, 2500, 900, 2500, 950, 2500, 950, 750, 950, 800, 950, 2500, 900, 2500, 950, 800, 950, 2450, 950, 2500, 950, 2500, 950, 750, 950, 800, 950, 2450, 950, 2500, 950, 800, 900, 800, 950, 2500, 950, 750, 950, 750, 950, 800, 950, 750, 950, 800, 900, 800, 950, 750, 950, 2500, 950, 750, 950, 800, 950, 750, 950, 800, 950, 750, 950, 750, 3550, 3350, 950, 2500, 900, 2500, 950, 2500, 950, 750, 950, 800, 950, 2500, 950, 2450, 950, 800, 950, 2450, 950, 2500, 950, 2500, 950, 750, 950, 800, 950, 2500, 900, 2500, 950, 800};
unsigned int codigoAirePana24[101] = {3500, 3350, 950, 2500, 950, 750, 950, 800, 900, 2500, 950, 800, 950, 2500, 900, 2500, 950, 800, 900, 2500, 950, 800, 950, 750, 950, 2500, 950, 750, 950, 2500, 950, 2500, 950, 750, 950, 750, 950, 2500, 950, 800, 900, 2500, 950, 800, 950, 750, 950, 750, 950, 800, 950, 750, 950, 2500, 950, 750, 1000, 2450, 950, 800, 900, 800, 950, 750, 950, 800, 3500, 3350, 950, 2500, 950, 750, 1000, 750, 900, 2500, 1000, 750, 950, 2500, 900, 2500, 1000, 750, 950, 2450, 950, 800, 950, 750, 950, 2500, 950, 750, 950, 2500, 950, 2500, 950, 750};
unsigned int codigoAirePana23[101] = {3500, 3350, 950, 800, 950, 750, 950, 800, 900, 2500, 950, 800, 950, 2450, 950, 2500, 950, 800, 900, 800, 950, 750, 950, 800, 950, 2500, 900, 800, 950, 2500, 900, 2500, 950, 800, 950, 750, 950, 2500, 950, 750, 950, 2500, 950, 750, 950, 800, 900, 800, 950, 750, 950, 800, 950, 2500, 900, 800, 950, 2500, 950, 750, 950, 750, 950, 800, 950, 750, 3550, 3350, 950, 750, 950, 750, 950, 800, 950, 2500, 900, 800, 950, 2500, 950, 2500, 900, 800, 950, 750, 950, 800, 900, 800, 950, 2500, 950, 750, 950, 2500, 950, 2500, 900, 800};
unsigned int codigoAirePana22[101] = {3500, 3350, 950, 2500, 950, 2500, 900, 2500, 950, 800, 950, 750, 950, 2500, 950, 2500, 900, 800, 950, 2500, 950, 2500, 900, 2500, 950, 800, 900, 800, 950, 2500, 950, 2500, 900, 800, 950, 750, 950, 2500, 950, 750, 1000, 2450, 950, 800, 900, 800, 950, 750, 950, 800, 900, 800, 950, 2500, 950, 750, 950, 2500, 950, 750, 950, 800, 900, 800, 950, 750, 3550, 3350, 950, 2500, 900, 2500, 1000, 2450, 950, 800, 900, 800, 950, 2500, 950, 2450, 950, 800, 950, 2500, 900, 2500, 950, 2500, 950, 750, 950, 800, 950, 2500, 900, 2500, 950, 800};
unsigned int codigoAirePana21[101] = {3500, 3350, 950, 750, 950, 2500, 950, 2500, 950, 750, 950, 800, 950, 2450, 950, 2500, 950, 800, 900, 800, 950, 2500, 950, 2450, 950, 800, 950, 750, 950, 2500, 950, 2500, 950, 750, 950, 750, 950, 2500, 950, 800, 900, 2500, 950, 800, 950, 750, 950, 750, 950, 800, 950, 750, 950, 2500, 950, 750, 950, 2500, 950, 750, 950, 800, 950, 750, 950, 800, 3500, 3350, 950, 800, 900, 2500, 950, 2500, 950, 750, 950, 800, 950, 2500, 900, 2500, 950, 800, 950, 750, 950, 2500, 950, 2500, 900, 800, 950, 750, 950, 2500, 950, 2500, 950, 750};
unsigned int codigoAirePana20[101] = {3550, 3350, 950, 2450, 950, 800, 950, 2500, 900, 800, 950, 750, 1000, 2450, 950, 2500, 950, 750, 950, 2500, 950, 750, 1000, 2450, 950, 750, 950, 800, 950, 2500, 900, 2500, 1000, 750, 950, 750, 950, 2500, 950, 750, 950, 2500, 950, 750, 950, 800, 950, 750, 950, 750, 950, 800, 950, 2500, 900, 800, 950, 2500, 950, 750, 950, 750, 1000, 750, 950, 750, 3550, 3350, 950, 2450, 950, 800, 950, 2500, 900, 800, 950, 750, 950, 2500, 950, 2500, 950, 750, 950, 2500, 950, 750, 1000, 2450, 950, 750, 950, 800, 950, 2500, 900, 2500, 950, 800};
unsigned int codigoAirePana19[101] = {3500, 3400, 900, 800, 950, 800, 900, 2500, 950, 800, 900, 800, 950, 2500, 900, 2550, 900, 800, 950, 750, 950, 800, 900, 2500, 950, 800, 900, 800, 950, 2500, 950, 2500, 900, 800, 950, 750, 950, 2500, 950, 800, 900, 2500, 950, 800, 900, 800, 950, 750, 950, 800, 900, 800, 950, 2500, 900, 800, 950, 2500, 950, 750, 950, 800, 900, 800, 950, 800, 3500, 3350, 950, 750, 950, 800, 900, 2550, 900, 800, 900, 800, 950, 2500, 950, 2500, 900, 800, 950, 800, 900, 800, 900, 2550, 900, 800, 950, 750, 950, 2500, 950, 2500, 950, 750};
unsigned int codigoAirePana18[101] = {3500, 3350, 950, 2500, 950, 2500, 950, 750, 950, 750, 950, 800, 950, 2500, 900, 2500, 950, 800, 950, 2450, 950, 2500, 950, 800, 900, 800, 950, 750, 950, 2500, 950, 2500, 950, 750, 1000, 750, 900, 2500, 950, 800, 950, 2500, 900, 800, 950, 750, 950, 800, 950, 750, 950, 750, 950, 2500, 1000, 700, 950, 2500, 950, 800, 900, 800, 950, 750, 950, 800, 3500, 3350, 950, 2500, 950, 2500, 950, 750, 950, 800, 900, 800, 950, 2500, 950, 2450, 950, 800, 950, 2500, 950, 2450, 1000, 750, 950, 750, 950, 750, 1000, 2450, 950, 2500, 950, 750};
unsigned int codigoAirePana17[101] = {3500, 3400, 900, 800, 950, 2500, 900, 800, 950, 750, 950, 800, 950, 2500, 900, 2500, 950, 800, 900, 800, 950, 2500, 950, 750, 950, 800, 900, 800, 950, 2500, 900, 2500, 950, 800, 950, 750, 950, 2500, 950, 750, 950, 2500, 950, 750, 950, 800, 950, 750, 950, 800, 900, 800, 950, 2500, 950, 750, 950, 2500, 950, 750, 950, 800, 900, 800, 950, 750, 3550, 3350, 950, 750, 950, 2500, 950, 750, 950, 800, 950, 750, 950, 2500, 950, 2500, 900, 800, 950, 750, 950, 2500, 950, 800, 900, 800, 950, 750, 1000, 2450, 950, 2500, 950, 750};
unsigned int codigoAirePana16[101] = {3500, 3400, 900, 2500, 950, 800, 900, 800, 950, 800, 900, 800, 950, 2500, 900, 2550, 900, 800, 900, 2550, 900, 800, 950, 750, 950, 800, 900, 800, 950, 2500, 900, 2550, 900, 800, 950, 750, 950, 2500, 950, 750, 950, 2500, 950, 800, 900, 800, 950, 750, 950, 800, 900, 800, 950, 2500, 900, 800, 950, 2500, 900, 800, 950, 800, 900, 800, 950, 750, 3550, 3350, 950, 2500, 900, 800, 950, 750, 1000, 750, 900, 800, 950, 2500, 950, 2500, 900, 800, 950, 2500, 900, 800, 950, 800, 900, 800, 950, 750, 950, 2500, 950, 2500, 950, 750};

void setup() {
  Serial.begin(19200);
  lcd.begin(16, 2);

  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);

  pinMode(sensorpir, INPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay2, HIGH);

  pinMode(relay3, OUTPUT);
  digitalWrite(relay3, HIGH);

  Serial2.begin(19200);
  SetUpWIFI() ;                   //Funcion para la configuracion del modulo wifi

  Serial1.begin(57600);

  if (finger.verifyPassword()) {                              //Condicion para comprobar que el sensor de huellas este correctamente comunicandose con el arduino
    //Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  //Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  //Serial.println("Waiting for valid finger..."); */
}

void loop() {
  // put your main code here, to run repeatedly:
  LimpiaDisplay();
  lcd.setCursor(1, 0); lcd.print("Ponga su dedo");

  WifiLoop();

  digitalWrite(TriggerPin, HIGH);   // genera el pulso de trigger por 10us
  delay(0.01);
  digitalWrite(TriggerPin, LOW);

  tiempo = pulseIn(EchoPin, HIGH);          // Lee el tiempo del Echo
  distancia = (tiempo / 2) / 29;          // calcula la distancia en centimetros
  delay(10);
  if (distancia <= 15 && distancia >= 2) {  // si la distancia es menor de 15cm
    getFingerprintIDez();
  }

  if(automatic){
    MotionSensor();
  }
  
}

//funciones para leer huellas
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      // Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      // Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
  //  Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
   // Serial.println("Did not find a match");
    return p;
  } else {
    //Serial.println("Unknown error");
    return p;
  }  
    // found a match!
    //Serial.print("ID # encontrado "); Serial.print(finger.fingerID);
    //Serial.print(" cona una confianza de "); Serial.println(finger.confidence);

    return finger.fingerID;

  }

  // returns -1 if failed, otherwise returns ID #
  int getFingerprintIDez() {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) {
      return -1;
    }

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) {
      return -1;
    }

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK) {
      return -1;
    }
    // found a match!
    //Serial.print("ID # encontrado "); Serial.print(finger.fingerID);
    //Serial.print(" con una confianza de "); Serial.println(finger.confidence);

    Lock();

    return finger.fingerID;

  }
  //fin de funciones para leer huellas

  //Inicio de funcion para Guardar huella

  uint8_t readnumber(void) {
    uint8_t num = 0;

    while (num == 0) {
      while (! Serial.available());
      num = Serial.parseInt();
    }
    return num;
  }

  void Registro() {
    //Serial.println("Ready to enroll a fingerprint!");
    //Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    finger.getTemplateCount();
    id = finger.templateCount + 1;
    if (id == 0) {// ID #0 not allowed, try again!
      return;
    }

    //Serial.println(finger.templateCount);
    LimpiaDisplay();
    lcd.setCursor(0,0); lcd.print("Registro");lcd.setCursor(0,1); lcd.print("Ponga su dedo");delay(500);
    //Serial.println(id);

    while (!  getFingerprintEnroll() );
  }
  uint8_t getFingerprintEnroll() {

    int p = -1;
    //Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
    while (p != FINGERPRINT_OK) {
      unsigned long currentWait = millis();
      if (primera) {
        previousWait = currentWait;
        primera = false;
      }

      p = finger.getImage();
      switch (p) {
        case FINGERPRINT_OK:
        LimpiaDisplay();
          lcd.setCursor(1, 0); lcd.print("imagen tomada");lcd.setCursor(1,1);lcd.print("Retire el dedo");delay(500);
          break;
        case FINGERPRINT_NOFINGER:
          //Serial.print(".");

          if (currentWait - previousWait > waitOff) {
            previousWait = currentWait;
            return p;
          }

          break;
        case FINGERPRINT_PACKETRECIEVEERR:
          //Serial.println("Communication error");
          break;
        case FINGERPRINT_IMAGEFAIL:
          //Serial.println("Imaging error");
          break;
        default:
          //Serial.println("Unknown error");
          break;
      }
    }

    // OK success!

    p = finger.image2Tz(1);
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image converted");
        break;
      case FINGERPRINT_IMAGEMESS:
        //Serial.println("Image too messy");
        return p;
      case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        return p;
      case FINGERPRINT_FEATUREFAIL:
        //Serial.println("Could not find fingerprint features");
        return p;
      case FINGERPRINT_INVALIDIMAGE:
        //Serial.println("Could not find fingerprint features");
        return p;
      default:
        //Serial.println("Unknown error");
        return p;
    }

    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
      p = finger.getImage();
    }
    //Serial.print("ID "); Serial.println(id);
    p = -1;

    LimpiaDisplay();
    lcd.setCursor(0,0); lcd.print("Vuelva a colocar");lcd.setCursor(0,1);lcd.print("el mismo dedo");delay(500);
  
    while (p != FINGERPRINT_OK) {
      unsigned long currentWait = millis();
      p = finger.getImage();
      switch (p) {
        case FINGERPRINT_OK:
          //Serial.println("Image taken");
          break;
        case FINGERPRINT_NOFINGER:
          //Serial.print(".");

          if (currentWait - previousWait > waitOff + 5000) {
            previousWait = currentWait;
            primera = true;
            return p;
          }

          break;
        case FINGERPRINT_PACKETRECIEVEERR:
          //Serial.println("Communication error");
          break;
        case FINGERPRINT_IMAGEFAIL:
          //Serial.println("Imaging error");
          break;
        default:
          //Serial.println("Unknown error");
          break;
      }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image converted");
        break;
      case FINGERPRINT_IMAGEMESS:
        // Serial.println("Image too messy");
        return p;
      case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        return p;
      case FINGERPRINT_FEATUREFAIL:
        //Serial.println("Could not find fingerprint features");
        return p;
      case FINGERPRINT_INVALIDIMAGE:
        //Serial.println("Could not find fingerprint features");
        return p;
      default:
        //Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    //Serial.print("Creating model for #");  Serial.println(id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK) {
      //Serial.println("Prints matched!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      //Serial.println("Communication error");
      return p;
    } else if (p == FINGERPRINT_ENROLLMISMATCH) {
      LimpiaDisplay();
      lcd.setCursor(0,1);lcd.print("sin coincidencia");
      //Serial.println("Fingerprints did not match");
      return p;
    } else {
      //Serial.println("Unknown error");
      return p;
    }

    //Serial.print("ID "); Serial.println(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
        LimpiaDisplay();
       lcd.setCursor(0,1);lcd.print("Registrado!");
      //Serial.println("Stored!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      //Serial.println("Communication error");
      return p;
    } else if (p == FINGERPRINT_BADLOCATION) {
      //Serial.println("Could not store in that location");
      return p;
    } else if (p == FINGERPRINT_FLASHERR) {
      //Serial.println("Error writing to flash");
      return p;
    } else {
      //Serial.println("Unknown error");
      return p;
    }
  }

  //Fin de funcion para guardar huella


  //Funciones del wifi
  void WifiLoop() {

    if (Serial2.available())
      // Lo que entra por WIFI à Serial
    {

      w = Serial2.read() ;
      Serial.print(w);
      W = W + w ;                    // Vamos montando un String con lo que entra
    }
    if (Serial.available())             // Lo que entra por Serial à WIFI
    { char s = Serial.read();
      Serial2.print(s);
    }
    if ( w == '\n')                     // Sin han pulsado intro
    {

      if ( W.indexOf("FOCO10") > 0 )
      {
        digitalWrite( relay2, LOW) ;
        Serial.println("'Foco 1 encendido'");

      }
      if ( W.indexOf("FOCO11") > 0 )
      {
        Serial.println("'Foco 1 apagado'");
        digitalWrite(relay2, HIGH);

      }
      if ( W.indexOf("FOCO20") > 0 )
      {
        digitalWrite(relay3, LOW);
        Serial.println("'Foco 2 encendido'");

      }
      if ( W.indexOf("FOCO21") > 0 )
      {
        Serial.println("'Foco 2 apagado'");
        digitalWrite(relay3, HIGH);

      }
      if ( W.indexOf("PUERTA") > 0 )
      {
        Lock();
      }
      if ( W.indexOf("REGISTRO") > 0 )
      {
        Registro();
      }
      /*if ( W.indexOf("PUERTADEL") > 0 )
      {
        //CODIGO DEL BORRADO DE CERRADURA
      }*/
      if ( W.indexOf("AIRE") > 0 )
      {
        AirePanaOnOff();
      }
      if ( W.indexOf("A24P") > 0 )
      {
        AirePana24();
      }
      if ( W.indexOf("A21P") > 0 )
      {
        AirePana21();
      }
      if (W.indexOf("A22P") > 0) {
        AirePana22();
      }
      if (W.indexOf("A20P") >  0) {
        AirePana20();
      }
      if ( W.indexOf("A18P") > 0 )
      {
        AirePana18();
      }

      if (W.indexOf("A16P") > 0) {
        AirePana16();
      }
      // dos comandosdistintos para activar/desactivar el sensor de movimiento
      if (W.indexOf("AUTO") > 0) {
        automatic = false;
      }
      if (W.indexOf("NOAUTO") > 0) {
        automatic = true;
      }

      //Un comando para desactivar/activar el sensor de movimiento
      /*if(W.indexOf("AUTO") > 0){
        if (automatic){
          automatic = false;  
        }else{
          automatic = true;
        }
      }*/
      
      W = "" ;  w = ' ' ;
      // Limpiamos las variables

    }
  }

  void SetUpWIFI()
  { String ordenes[] =
    { //"AT+RST",
      "AT+CWMODE=3",
      //"AT+CWQAP",
      //AT"AT+CWLAP",
      "AT+CWJAP=\"dlink\",\"\"",
      "AT+CIFSR" ,
      "AT+CIPMUX=1",
      "AT+CIPSERVER=1,80",
      "END"        // Para reconocer el fin de los comandos AT
    };
    int index = 0;
    while (ordenes[index] != "END")
    { Serial2.println(ordenes[index++]);
      while ( true)
      { String s = GetLineWIFI();
        if ( s != "") Serial.println(s);
        if ( s.startsWith("no change"))
          break;
        if ( s.startsWith("OK"))
          break;
        if ( s.startsWith("ready"))
          break;
        if (s.startsWith("FAIL"))
          break;
        // if (millis()-T >10000) break;
      }
      Serial.println("....................");
    }
  }

  String GetLineWIFI()
  { String S = "" ;
    if (Serial2.available())
    { char c = Serial2.read(); ;
      while ( c != '\n' )            //Hasta que el caracter sea intro
      { S = S + c ;
        delay(25) ;
        c = Serial2.read();
      }
      return ( S ) ;
    }
  }
  //fin wifi

  //funcion para activar la cerradura con reles de logica inversa xd
  void Lock() {
    //Serial.print("\nCerradura abierta");
    lcd.setCursor(0, 1);
    lcd.print("PUERTA ABIERTA");
    digitalWrite(relay, LOW);
    delay(3000);
    digitalWrite(relay, HIGH);
  }
  //fin de funcion de cerradura

  //Inicio de senales IR
  void EnviarIR(unsigned long codigo) {
    irsend.sendNEC(codigo, 32);
    delay(100);
  }
  void RecivirIR() {
    if (irrecv.decode(&results)) {
      //Serial.println(results.value, HEX);
      irrecv.resume();
    }
  }
  //Fin dee IR

  //funcion temporizador
  void MotionSensor() {
    unsigned long currentMillis = millis();
    if (digitalRead(sensorpir) == HIGH) {
      //Serial.println("Detectado movimiento por el sensor");
      previousMillis = currentMillis;
      relayState = LOW;
      if(n == 0){
        AirePanaOnOff();
      }
      n = 1;
    } else {

      if (currentMillis - previousMillis > intervalOff) {
        previousMillis = currentMillis;
        relayState = HIGH;
        AirePanaOnOff();
        n = 0;
      }
    }
    /*Serial.println(currentMillis);
      Serial.print("Previo: ");
      Serial.println(previousMillis);
      delay(1000);*/
    digitalWrite(relay2, relayState);

  }
  //fin temporizador

  //mensaje error lcd
  void LimpiaDisplay()
  {
    for (int i = 0; i < 16; i++) {
      lcd.setCursor(i, 0);
      lcd.print(" ");
      lcd.setCursor(i, 1);
      lcd.print(" ");
    }
  }

  void NoAcceso()
  {
    lcd.setCursor(0, 1);
    lcd.print("ACCESO DENEGADO");
  }

  //

  //Funciones codigos RAW del Aire
  void AirePanaOnOff(){
  irsend.sendRaw(codigoAirePanaOn20, 101, 38);
  }
  void AirePanaOn() {
    irsend.sendRaw(codigoAirePanaOn, 101, 38);
  }
  void AirePana24() {
    irsend.sendRaw(codigoAirePana24, 101, 38);
  }
  void AirePana23() {
    irsend.sendRaw(codigoAirePana23, 101, 38);
  }
  void AirePana22() {
    irsend.sendRaw(codigoAirePana22, 101, 38);
  }
  void AirePana21() {
    irsend.sendRaw(codigoAirePana21, 101, 38);;
  }
  void AirePana20() {
    irsend.sendRaw(codigoAirePana20, 101, 38);
  }
  void AirePana19() {
    irsend.sendRaw(codigoAirePana19, 101, 38);
  }
  void AirePana18() {
    irsend.sendRaw(codigoAirePana18, 101, 38);
  }
  void AirePana17() {
    irsend.sendRaw(codigoAirePana17, 101, 38);
  }
  void AirePana16() {
    irsend.sendRaw(codigoAirePana16, 101, 38);
  }

  //Fin de funciones de codigos RAW