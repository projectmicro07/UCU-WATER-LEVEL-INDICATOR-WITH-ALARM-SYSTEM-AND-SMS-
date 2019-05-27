
#define buzzer 14
#define led 31
#define led1 30
#define led2 29
#define led3 28
#include <LiquidCrystal.h>

int modem_response;
int PWON= A5;
char aux_string[30];
char* owner = "09272358040";

int sound = 250;
LiquidCrystal lcd(9,8,7,6,5,4);

const int trigPin = 15;
const int echoPin = 16;
const char lock1 = 2;
const char lock2 = 3;


const int gsmTimeout = 30;
const int gsmMaxBuffer = 128;
char gsmBuffer[gsmMaxBuffer] = "";

void setup() {

  pinMode(lock1,OUTPUT);
  pinMode(lock2,OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  lcd.begin (16,2);
  lcd.print ("H2OLvL indicator");
  lcd.setCursor(0,1);
  lcd.print ("Project MICRO");
  delay(100);
  
    Serial1.begin(9600);
    Serial.begin(9600);    

    AUTO_PWON();
    Connect();
    delay(100);
    FireUpModem();
  
}
void loop() {

  long duration, distance;
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;

  Serial.print("DISTANCE = ");
  Serial.println(distance);

  CheckForMessages();
  
if (distance > 40 || distance <= 30){
    digitalWrite(led, LOW);
  }
  else {
    lcd.setCursor(0,0);
    lcd.print("Warning level 1 ");
    lcd.setCursor(0,1);
    lcd.print("WaterLvL(cm): ");
    lcd.print(distance);
    delay(10);
    digitalWrite(led, HIGH);
  }
  if (distance > 30 || distance <= 20){
    digitalWrite(led1, LOW);
  }
  else {
    lcd.setCursor(0,0);
    lcd.print("Warning level 2 ");
    lcd.setCursor(0,1);
    lcd.print("WaterLvL(cm): ");
    lcd.print(distance);
    delay(10);
    digitalWrite(led1, HIGH);
  }
  if (distance > 20 || distance <= 9){
    digitalWrite(led2, LOW);
    noTone(buzzer);
  }
  else {
    lcd.setCursor(0,0);
    lcd.print("Warning level 3 ");
    lcd.setCursor(0,1);
    lcd.print("WaterLvL(cm): ");
    lcd.print(distance);
    delay(10);
    tone(buzzer, sound);
    delay(50);
    digitalWrite(led2, HIGH);
  }
  if (distance > 9 || distance <= 0){
    digitalWrite(led3, LOW);
    noTone(buzzer); 
   
      
  }
  else {
    lcd.setCursor(0,0);
    lcd.print("Warning level 4 ");
    lcd.setCursor(0,1);
    lcd.print(" WaterLvL(cm): ");
    lcd.print(distance);
    delay(10);
    tone(buzzer, sound);
    digitalWrite(led3, HIGH); 
    SendMessage("WATER LEVEL WARNING LEVEL4. FORCE EVACUATION!",owner);
    delay(100);
   
  }
     delay(100);
}

void CheckForMessages()
{
   if(Serial1.available()){
     byte charsRead = Serial1.readBytesUntil('\n',gsmBuffer,gsmMaxBuffer);
     if(charsRead){
       gsmBuffer[charsRead] = 0; //Terminate String
       _gsmSerialHandleLine(String(gsmBuffer));
       
     }
   }
}


boolean _isNewSms(const String &line)
{
  // +CMTI: "SM",11
  return line.indexOf("+CMTI") == 0 &&
         line.length() > 12;
}

boolean _isSms(const String &line)
{
  return line.indexOf("+CMGR") == 0;
}

void _receiveTextMessage(const String &line)
{
  String index = line.substring(12);
  index.trim();
  Serial1.flush();
  Serial1.print("AT+CMGR=" + index + "\r");
}

boolean _gsmReadBytesOrDisplayError(char numberOfBytes)
{
  if(!_gsmWaitForBytes(numberOfBytes, gsmTimeout))
  {
     Serial.println("Error Reading From Device");
    return false;
  }

  while(Serial1.available())
  {
    char next = Serial1.read();
   
  }
  
  return true;
}

char _gsmWaitForBytes(char numberOfBytes, int timeout)
{

  while(Serial1.available() < numberOfBytes)
  {
    delay(200);
    timeout -= 1;
    if(timeout == 0)
    {
      return 0;
    }
  }
  return 1;
}

boolean _readAndPrintSms()
{
  // Line contains the +CMGR response, now we need to read the message
  
  byte charsRead = Serial1.readBytesUntil('\n', gsmBuffer, gsmMaxBuffer);
  
  if(charsRead > 1)
  {
    charsRead--; // Skip newline
    gsmBuffer[charsRead] = 0; // Terminate string
    _gsmReadBytesOrDisplayError(4); // OK\r\n
    Serial.println(gsmBuffer);
    
  }
}

void _gsmSerialHandleLine(const String &s)
{
  if(_isNewSms(s))
  {
      _receiveTextMessage(s);
  }
  else if(_isSms(s))
  {
    _readAndPrintSms();
  }
  else
  {
    
  }
}

int SendMessage(char* message, char* number)
{
  sprintf(aux_string,"AT+CMGS=\"%s\"", number);
  modem_response = SendModemCommand(aux_string, ">", 2000); 
  if(modem_response == 1)
   {
     Serial1.print(message);
     Serial1.write(0x1A);
     modem_response = SendModemCommand("","OK",20000);
     if(modem_response == 1)
      {
        Serial.println("Message Sent");
      }else
      {
        Serial.println("Message not Sent");
      }
   } 
}

void AUTO_PWON(){

    int answer=0;
    
    answer = SendModemCommand("AT", "OK", 2000);
    if (answer == 0)
    {
       
        digitalWrite(PWON,HIGH);
        delay(3000);
        digitalWrite(PWON,LOW);
    
       
        while(answer == 0){   
            answer = SendModemCommand("AT", "OK", 2000);    
        }
    }
    
}

void FireUpModem(){
  
  Serial.println("Initializing Modem");
  SendModemCommand("AT+CMGDA=\"DEL ALL\"","OK",10000);
  SendModemCommand("AT+CNMI=3,1","OK",1000);
  SendModemCommand("AT+CMGF=1","OK",1000);
  Serial.println("Modem Initialization OK");
}

void Connect()
{
    Serial.println("Connecting to nearest cell site");

    while( (SendModemCommand("AT+CREG?", "+CREG: 0,1", 500) || SendModemCommand("AT+CREG?", "+CREG: 0,5", 500)) == 0 ); 
    Serial.println("Connected");
}


int SendModemCommand(char* cmd, char* resp, unsigned int tout)
{
    int ctr=0;
    int ans=0;
    char buff[100];
    unsigned long prev;

    memset(buff, '\0', 100);
    
    delay(100);
    
    while( Serial1.available() > 0) Serial1.read(); 
    
    Serial1.println(cmd);    


    ctr = 0;
    prev = millis();


    do{

        if(Serial1.available() != 0){    
            buff[ctr] = Serial1.read();
            ctr++;
             if (strstr(buff, resp) != NULL)    
            {
                ans = 1;
            }
        }

    }while((ans == 0) && ((millis() - prev) < tout));    

    return ans;
}
