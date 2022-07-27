#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

const int oneWireBus = 4;

//Vairables para sensor de polvo
const int polvo = 12;
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

/* 1. Define the WiFi credentials */
//#define WIFI_SSID "UNIDAD_2 MAESTROS"
//#define WIFI_PASSWORD "7677303061"
//#define WIFI_SSID "HOME-E237"
//#define WIFI_PASSWORD "B7C02F52D872BCEF"
#define WIFI_SSID "CC_Oficinas"
#define WIFI_PASSWORD "Tns01454"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyBBQW7bgWsT7JKkZ6CSiIc6zkQZADMBNhg"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://esp-app-da2f9-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
/*#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"*/

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int intValue;
bool signupOK = false;

OneWire oneWire(oneWireBus);

DallasTemperature sensors(&oneWire);

void setup(){
  pinMode(2, OUTPUT);
  
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
   sendDataPrevMillis = millis();
   if(Firebase.RTDB.getInt(&fbdo, "aire1/switch")) {
    intValue = fbdo.intData();
    if(intValue == 0) {
      digitalWrite(2, LOW);
    } else {
      digitalWrite(2, HIGH);
    }
  }

  //Mandar datos del sensor de temperatura
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);
  if(Firebase.RTDB.getFloat(&fbdo, "aire1/temperatura")) {
    Firebase.RTDB.setFloat(&fbdo, "aire1/temperatura", temperatura);
  }

  //Mandar datos del sensor de polvo
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(polvo);
  delayMicroseconds(deltaTime);
  delayMicroseconds(sleepTime);

  calcVoltage = voMeasured * (5.0 / 1024.0);

  dustDensity = 170 * calcVoltage - 0.1;

  Serial.println(dustDensity);
  delay(1000);

  if(Firebase.RTDB.getFloat(&fbdo, "aire1/polvo")) {
    Firebase.RTDB.setFloat(&fbdo, "aire1/polvo", polvo);
  }
 }
}
