#include <FirebaseArduino.h>         //cloud library
#include <ESP8266WiFi.h>             //wifi library
#include <ArduinoJson.h>

#define WIFI_SSID "Your WiFi network"             //replace SSID with your wifi username
#define WIFI_PASSWORD "Your WiFi password"          //replace PWD with your wifi password
#define WIFI_LED D8                  //connect a led to any of the gpio pins of the board and replace pin_number with it eg. D4                      

#define FIREBASE_HOST "Your Firebase host"                         //link of api
#define FIREBASE_AUTH "Your Firebase auth"           //database passcode

#include "DHT.h" 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DHTPIN 2     // what pin we're connected to

#include <Adafruit_NeoPixel.h>
#define NEOPIXEL_TYPE NEO_RGB + NEO_KHZ800
#define RGB_PIN 5 

#define ALARM_COLD 17.0
#define ALARM_HOT 25.0
#define WARN_COLD 19.0
#define WARN_HOT 22.0

unsigned char r = 0; // LED RED value
unsigned char g = 0; // LED Green value
unsigned char b = 0; // LED Blue value

#include <ESP8266HTTPClient.h>

////////////////////////////////////////////////////////////////////////

const String path = "/room/light/status";
const String control = "/room/control/status";
const String Temp = "/room/setTemp";

bool printOnce = true;
int previousTemp = 0;

DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, RGB_PIN, NEOPIXEL_TYPE);

int redLed_dht22 = 4;
int greenLed_dht22 = 12;
float tim;
int redLed_mq2 = 16;
//int greenLed_mq2 = 5;
int buzzer = 13;
int smokeA0 = A0;
// Your threshold value
int sensorThres = 500;
unsigned long beginTime = 0;
unsigned long passedTime = 0;
unsigned long maxmillis = 4294967296;
unsigned long timeToSend = 900000;

unsigned long hysteresis = 30000;
unsigned long lastOff = 0;

bool Present = true;

float h = 0;
float t = 0;

//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 30;        

//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;  

boolean lockLow = true;
boolean takeLowTime;  

int pirPin = 14;    //the digital pin connected to the PIR sensor's output

bool firstIn;

void setup() {
  Serial.begin(115200);
  pinMode(WIFI_LED,OUTPUT);                         //define pinmodes
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  
  //connect to wifi
//  while (WiFi.status() != WL_CONNECTED) {           //wait till connected to WiFi
//    delay(100);  
//    digitalWrite(WIFI_LED,LOW);                     //Blink the light till connected to WiFi
//    delay(100);
//    digitalWrite(WIFI_LED,HIGH);
//    Serial.print("."); }

  while(WiFi.status() != WL_CONNECTED) {
      Serial.println("Connecting to WiFi ...");
      delay(100);
  }
    
  Serial.println("");
  Serial.println("WiFi connected");
  //digitalWrite(WIFI_LED,HIGH);  // Check another data pin for WIFI_LED indication
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);             //connect to Database
  delay(1000);
  
/////////////////////////////////////////////////////////////

  Serial.println("DHT test");
  dht.begin(); 
  pinMode(redLed_dht22, OUTPUT);
  pinMode(greenLed_dht22, OUTPUT); 
  pinMode(redLed_mq2, OUTPUT);
  pinMode(RGB_PIN, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(smokeA0, INPUT);
  //Motion
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);
    Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);
  firstIn = true;

     h = dht.readHumidity();
     t = dht.readTemperature();
            // check if returns are valid, if they are NaN (not a number) then something went wrong!
      if (isnan(t) || isnan(h)) {
        Serial.println("Failed to read from DHT");
      } else {
        Serial.print("Time: ");
        tim = millis()/1000;
        Serial.print(tim);
        Serial.print(" secs\t");
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" *C");
      }
}

void loop() {
  
  String firebaseResult = "";
  int tempSet = Firebase.getInt(Temp);
  int lightControl = Firebase.getInt(control);
  
  //Serial.println(firebaseResult);
  if (Firebase.failed()) {
      Serial.println("Firebase get failed");
      Serial.println(Firebase.error());
      return;
  }
  //String statos = firebaseResult.get(path);
  if(tempSet != previousTemp){
    
    Serial.println(tempSet);
    previousTemp = tempSet;
  }
  //Serial.println(firebaseResult);
//  delay(100);

  if (lightControl){
          
          firebaseResult = Firebase.getString(path);
          
          if (firebaseResult=="ON"){
          //code to happen if the status is ON  
          digitalWrite(WIFI_LED,HIGH);   
      }else{
          //code to happen if the status is OFF
          digitalWrite(WIFI_LED,LOW);          
        }        
  }else{
    
  }

  if(digitalRead(pirPin) == HIGH){
       if(lockLow){  
         //wait for a transition to LOW
         lockLow = false;
         Present = true;            
         Serial.println("---");
         Serial.print("motion detected at ");
         Serial.print(millis()/1000);
         Serial.println(" sec");
//////////////////////////////

      if (WiFi.status() == WL_CONNECTED){

          //motion data
          DynamicJsonBuffer JSONmotion;   //Declaring static JSON buffer
          JsonObject& JSONencoderm = JSONmotion.createObject();
          JsonObject& value = JSONencoderm.createNestedObject("presence"); //JSON array
          value["value"]=1;
          String messagem;
          JSONencoderm.printTo(messagem);
          //Serial.println(messagem);

          HTTPClient http;    //Declare object of class HTTPClient
          
          http.begin("your api url");      //Specify request destination
          http.addHeader("Content-Type", "application/json");  //Specify content-type header
          // 
          int httpCode = http.POST(messagem);   //Send the request
          String payload = http.getString();   //Get the response payload
          // 
          Serial.println(httpCode);   //Print HTTP return code
          Serial.println(payload);    //Print request response payload
           
          http.end();  //Close connection
           }else {
            Serial.println("Error in WiFi connection");
          }
          
         }         
         takeLowTime = true;
       }

     if(digitalRead(pirPin) == LOW){

       if(takeLowTime){
        lowIn = millis();          //save the time from high to LOW
        takeLowTime = false;       //only done at the start of a LOW phase
        }
 
       //assume no more motion is going to happen after the given pause
       if(!lockLow && millis() - lowIn > pause){  
           //only executed again after a new motion sequence has been detected
           lockLow = true;
           Present = false;                        
           Serial.print("motion ended at ");      //output
           Serial.print((millis() - pause)/1000);
           Serial.println(" sec");
           lastOff = (millis() - pause);
/////////////////////////////////////////////
     
      if (WiFi.status() == WL_CONNECTED){
        
          DynamicJsonBuffer JSONmotiond;   //Declaring static JSON buffer
          JsonObject& JSONencodermd = JSONmotiond.createObject();
          JsonObject& value = JSONencodermd.createNestedObject("presence"); //JSON array
          value["value"]=0;
          String messagemd;
          JSONencodermd.printTo(messagemd);
          //Serial.println(messagemd);

          HTTPClient http;    //Declare object of class HTTPClient
          
          http.begin("your api url");      //Specify request destination
          http.addHeader("Content-Type", "application/json");  //Specify content-type header
          // 
          int httpCode = http.POST(messagemd);   //Send the request
          String payload = http.getString();   //Get the response payload
          // 
          Serial.println(httpCode);   //Print HTTP return code
          Serial.println(payload);    //Print request response payload
           
          http.end();  //Close connection
           }else {
            Serial.println("Error in WiFi connection");
          }
           
           }
       }

        // Turn red LED on and green LED off if the temperature is 25 degrees or more.
      // Turn green LED on and red LED off if the temperature is less than 25 degrees.

      if (Present){

            if (t >= 20) {
              digitalWrite(redLed_dht22, HIGH);
              digitalWrite(greenLed_dht22, LOW);
      
              // Neon LED Option
              b = (tempSet < ALARM_COLD) ? 255 : ((tempSet < WARN_COLD) ? 150 : 0);
              r = (tempSet >= ALARM_HOT) ? 255 : ((tempSet > WARN_HOT) ? 150 : 0);
              g = (tempSet > ALARM_COLD) ? ((tempSet <= WARN_HOT) ? 255 : ((tempSet < ALARM_HOT) ? 150 : 0)) : 0;
              pixel.setPixelColor(0, r, g, b);
              pixel.show();
            }
            else {
              digitalWrite(redLed_dht22, LOW);
              digitalWrite(greenLed_dht22, HIGH);
              pixel.setPixelColor(0, 0, 0, 0);
              pixel.show();
            }

            if(!lightControl){digitalWrite(WIFI_LED,HIGH);}
            
        }else{
                     
                        if((millis()- lastOff) > hysteresis){
                                digitalWrite(redLed_dht22, LOW);
                                digitalWrite(greenLed_dht22, LOW);
                                if(!lightControl){digitalWrite(WIFI_LED,LOW);}
                                pixel.setPixelColor(0, 0, 0, 0);
                                pixel.show();
                          }else{
          
                                  if (t >= 20) {
                                    digitalWrite(redLed_dht22, HIGH);
                                    digitalWrite(greenLed_dht22, LOW);
                            
                                    // Neon LED Option
                                    b = (tempSet < ALARM_COLD) ? 255 : ((tempSet < WARN_COLD) ? 150 : 0);
                                    r = (tempSet >= ALARM_HOT) ? 255 : ((tempSet > WARN_HOT) ? 150 : 0);
                                    g = (tempSet > ALARM_COLD) ? ((tempSet <= WARN_HOT) ? 255 : ((tempSet < ALARM_HOT) ? 150 : 0)) : 0;
                                    pixel.setPixelColor(0, r, g, b);
                                    pixel.show();
                                  }
                                  else {
                                    digitalWrite(WIFI_LED,HIGH);
                                    digitalWrite(redLed_dht22, LOW);
                                    digitalWrite(greenLed_dht22, HIGH);
                                    pixel.setPixelColor(0, 0, 0, 0);
                                    pixel.show();
                                  }
                      
                                  if(!lightControl){digitalWrite(WIFI_LED,HIGH);}
                  
                  }
          }

    // get the time the program started
    if(firstIn){
      beginTime = millis();

      // set first reading false to iniciate again only after the humidity and temperature datas are sent
      firstIn = false; 
    }
    if((int)(millis() - beginTime)>=0){
      passedTime = millis() - beginTime;
    }

    // when millis() rolls over (after aproximatelly 50 days)
    else{ 
      passedTime = maxmillis + millis() - beginTime;
    }

    if(passedTime >= timeToSend){
      firstIn = true;
        // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      h = dht.readHumidity();
      t = dht.readTemperature();

      // check if returns are valid, if they are NaN (not a number) then something went wrong!
      if (isnan(t) || isnan(h)) {
        Serial.println("Failed to read from DHT");
      } else {
        Serial.print("Time: ");
        tim = millis()/1000;
        Serial.print(tim);
        Serial.print(" secs\t");
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" *C");
      }
     

      if (WiFi.status() == WL_CONNECTED){

          //temperature data
          DynamicJsonBuffer JSONbuffer;   //Declaring static JSON buffer
          JsonObject& JSONencoder = JSONbuffer.createObject();
          JsonObject& value = JSONencoder.createNestedObject("temperature"); //JSON array
          value["value"]=t;
          String message;
          JSONencoder.printTo(message);
          Serial.println(message);

          HTTPClient http;    //Declare object of class HTTPClient
          
          http.begin("your api url");      //Specify request destination
          http.addHeader("Content-Type", "application/json");  //Specify content-type header
          // 
          int httpCode = http.POST(message);   //Send the request
          String payload = http.getString();   //Get the response payload
          // 
          Serial.println(httpCode);   //Print HTTP return code
          Serial.println(payload);    //Print request response payload


          //humidity data
          DynamicJsonBuffer JSONHum;   //Declaring static JSON buffer
          JsonObject& JSONencoderh = JSONHum.createObject();
          JsonObject& values = JSONencoderh.createNestedObject("humidity"); //JSON array
          values["value"]=h;
          String messageh;
          JSONencoderh.printTo(messageh);
          Serial.println(messageh);

          httpCode = http.POST(messageh);   //Send the request
          payload = http.getString();   //Get the response payload
          // 
          Serial.println(httpCode);   //Print HTTP return code
          Serial.println(payload);
           
          http.end();  //Close connection
           }else {
            Serial.println("Error in WiFi connection");
          }
         
    }

//  // Wait 5 seconds before starting the next reading.
//  delay(5000);
  
  int analogSensor = analogRead(smokeA0);

 
  // Checks if it has reached the threshold value
  if (analogSensor > sensorThres)
  {
    digitalWrite(redLed_mq2, HIGH);
    //digitalWrite(greenLed_mq2, LOW);
    tone(buzzer, 100, 100);
    Serial.print("Pin A0: ");
    Serial.println(analogSensor);
     if (WiFi.status() == WL_CONNECTED){

          //temperature data
          DynamicJsonBuffer JSONbuffers;   //Declaring static JSON buffer
          JsonObject& JSONencoders = JSONbuffers.createObject();
          JsonObject& value = JSONencoders.createNestedObject("smoke"); //JSON array
          value["value"]=1;
          String messages;
          JSONencoders.printTo(messages);
          Serial.println(messages);

          HTTPClient http;    //Declare object of class HTTPClient
          
          http.begin("your api url");      //Specify request destination
          http.addHeader("Content-Type", "application/json");  //Specify content-type header
          // 
          int httpCode = http.POST(messages);   //Send the request
          String payload = http.getString();   //Get the response payload
          // 
          Serial.println(httpCode);   //Print HTTP return code
          Serial.println(payload);    //Print request response payload
          
          http.end();  //Close connection
           }else {
            Serial.println("Error in WiFi connection");
          }
  }
  else
  {
    digitalWrite(redLed_mq2, LOW);
    //digitalWrite(greenLed_mq2, HIGH);
    noTone(buzzer);
  }
//  delay(100);
  

} 
