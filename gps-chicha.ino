

/*
   GPS-CHICHA
   v0.01: SMS Sending - [DONE].

   ESP8266 ESP-12S A9/A9G with ESPSoftwareSerial messaging via Interrupts, Hardware ESP8266  Watchdog restart.
   Battery 3.7v MUST be connected to use GSM/SMS!
*/


//Use this ESP SoftwareSerial: https://github.com/plerup/espsoftwareserial
#include <SoftwareSerial.h>
//#include <EEPROM.h>

SoftwareSerial gsmSerial;

String OwnerPhone = "+79991234567";
String TestMessage = "Privet Chicha! This is the SMS test.";


//constants:
const int A9PowerPin = 16;
const int A9ResetPin = 15;
const int A9RXPin = 14;
const int A9TXPin = 12;
const int A9LowPowerPin = 2;
const int A9BaudRate = 19200;
const int BaudRate = 57600;
const int WatchdogTimeout = 5000;

boolean debug = true;
boolean GSMOperational = false;

//ПЗУ
const int OperationEepromAddress = 1;
const int SMSOperationEepromAddress = 2;
const int InitEepromAddress = 3;
const int MessageNotifyEepromAddress = 4;
const int SMSEepromAddress = 100;
const int MessageEepromAddress = 200;


//randomSeed(analogRead(A9RXPin));
int Password = random(9999);

String grepValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void RespondToCallOrSMS(String response) {

  //if (response.substring(1, 5) == "CLIP" or response.substring(1, 4) == "CMT" or response.substring(0, 9) == "RING+CLIP") {
  //if (response.substring(0, 9) == "RING+CLIP") {
  if (response.substring(1, 4) == "CMT") {
    String TempPhone = grepValue(response, '"', 1);
    TempPhone.reserve(24);
    if (TempPhone == OwnerPhone) {
      if (response.endsWith("on") or response.endsWith("On") or response.endsWith("ON"))  {

        if (debug)
        {
          Serial.println("Received ON via SMS.");
        }

      } else if (response.endsWith("off") or response.endsWith("Off") or response.endsWith("OFF")) {
        if (debug)
        {
          Serial.println("Received OFF via SMS.");
        }
      }
    } else {
      if (response.endsWith(String(Password))) {
        if (debug)
        {
          Serial.println("Received PASSWORD via SMS.");
        }
      }
    }
  }
}

void A9SwitchOn()
{
  GSMOperational = false;

  String message = "";
  message = ATCommand("AT", 1000);
  if ((message.indexOf("OK") >= 0) || (message.indexOf("0") >= 0))
  {
    if (debug)
    {
      Serial.println("GSM chip is awailable.");
    }
    GSMOperational = true;
  } else {
    if (debug)
    {
      Serial.println("GSM chip is NOT awailable.");
    }
  }

  if (GSMOperational == false) {

    digitalWrite(A9ResetPin, LOW);
    digitalWrite(A9LowPowerPin, HIGH);
    digitalWrite(A9PowerPin, LOW);
    sleep(3000);
    digitalWrite(A9PowerPin, HIGH);
    sleep(100);

    while (GSMOperational != true)
    {
      sleep(100);

      message = ATCommand("AT", 1000);
      if ((message.indexOf("OK") >= 0) || (message.indexOf("0") >= 0))
      {
        if (debug)
        {
          Serial.println("A9/A9G is awailable and operational.");
        }
        GSMOperational = true;
      } else {
        sleep(1000);
      }
    }
  }
}

bool CheckATReplyExists(String Command, String Needle, int Timeout) {

  if ( sizeof(Timeout) == 0 ) {
    Timeout = 1000;
  }
  String message = "";
  message = ATCommand(Command, Timeout);
  if (message.indexOf(Needle) >= 0)
  {
    return true;
  } else {
    return false;
  }

}


bool CheckGSMRegisteredOK() {
  String message = "";
  message = ATCommand("AT+CREG?", 3000);
  //BAD: +CREG: 0 || +CREG: 1,0 || +CREG: 1,2 || +CREG: 1,3 || +CREG: 2 || +CREG: 3 || +CREG: 4 || +CREG: 8 || +CREG: 9 || +CREG: 10
  if ((message.indexOf("+CREG: 0") >= 0) || (message.indexOf("+CREG: 1,0") >= 0) || (message.indexOf("+CREG: 1,2") >= 0) || (message.indexOf("+CREG: 1,3") >= 0)  || (message.indexOf("+CREG: 2") >= 0) || (message.indexOf("+CREG: 3") >= 0) || (message.indexOf("+CREG: 4") >= 0) || (message.indexOf("+CREG: 8") >= 0) || (message.indexOf("+CREG: 9") >= 0) || (message.indexOf("+CREG: 10") >= 0))
  {
    if ((message.indexOf("+CREG: 1,0") >= 0) || (message.indexOf("+CREG: 1,2") >= 0)) {
      if (debug)
      {
        Serial.println("GSM Problem: Did you connect 3.7v cable to the GSM board?");
      }
    }
    if (message.indexOf("+CREG: 2") >= 0) {
      if (debug)
      {
        Serial.println("GSM Problem: Searching for GSM network...");
      }
    }
    if (message.indexOf("+CREG: 1,3") >= 0) {
      if (debug)
      {
        Serial.println("GSM Problem: No SIM card.");
      }
    }
    if (message.indexOf("+CREG: 3") >= 0) {
      if (debug)
      {
        Serial.println("GSM Problem: registration denied.");
      }
    }
    if (message.indexOf("+CREG: 4") >= 0) {
      if (debug)
      {
        Serial.println("GSM Problem: Unknown problem (CODE 4: out of GERAN/UTRAN/E-UTRAN coverage).");
      }
    }
    if (message.indexOf("+CREG: 8") >= 0) {
      if (debug)
      {
        Serial.println("GSM Problem: emergency services only.");
      }
    }
    if (message.indexOf("+CREG: 9") >= 0) {
      if (debug)
      {
        Serial.println("GSM Problem: registered for CSFB not preferred, home network.");
      }
    }
    if (message.indexOf("+CREG: 10") >= 0) {
      if (debug)
      {
        Serial.println("GSM Problem: registered for CSFB not preferred, roaming network.");
      }
    }
    return false;
  } else {
    if (message.indexOf("+CREG: 1,1") >= 0) {
      if (debug)
      {
        Serial.println("GSM OK: registered, home network.");
      }
    }
    if (message.indexOf("+CREG: 5") >= 0) {
      if (debug)
      {
        Serial.println("GSM OK: registered, roaming.");
      }
    }
    if (message.indexOf("+CREG: 6") >= 0) {
      if (debug)
      {
        Serial.println("GSM OK: registered for SMS only, home network");
      }
    }
    if (message.indexOf("+CREG: 7") >= 0) {
      if (debug)
      {
        Serial.println("GSM OK: registered for SMS only, roaming");
      }
    }
    return true;
  }
}


void sleep(int sleep_time) {
  //non blocking delay(sleep_time);
  long int time = millis();
  while ((time + sleep_time) > millis()) {

    //do some useful background tasks:

    //reset watchdog
    ESP.wdtFeed();

    //do background esp8266 tasks
    yield();

  }
}

String ATCommand(String command, const int timeout)
{

  String response = "";   // a String to hold incoming data
  if (debug)
  {
    Serial.print("Command Sent: ");  Serial.println(command);
  }
  gsmSerial.println(command);

  long int time = millis();
  while ((time + timeout) > millis())
  {
    //do background esp8266 tasks
    yield();

    //reset watchdog
    ESP.wdtFeed();

    while (gsmSerial.available())
    {
      char c = gsmSerial.read();
      response += c;
    }
  }
  if (debug)
  {
    Serial.print("A9G Response: ");
    Serial.println(response);
  }
  return response;
}

void GetGPSLocation() {
	ATCommand("AT+LOCATION=2", 3000);
}

void GSMInit() {
  if (!CheckATReplyExists("AT+IPR?", String(A9BaudRate), 3000)) {
    ATCommand("AT+IPR=" + String(A9BaudRate) + "\"", 3000);
  }

  //full functionality
  //ATCommand("AT+CFUN=1", 1000);

  //disable echo
  ATCommand("ATE0", 1000);
  ATCommand("ATV1", 1000);
  
  //show full cmee errors (2) instead of codes (1)
  ATCommand("AT+CMEE=2", 1000);

  //Switch text mode in SMS
  if (!CheckATReplyExists("AT+CMGF?", "+CMGF: 1", 3000)) {
    ATCommand("AT+CMGF=1", 3000);
  }

  //Switch CallerID ON "+CLIP:1,1"
  if (!CheckATReplyExists("AT+CLIP?", "+CLIP:1,1", 3000)) {
    ATCommand("AT+CLIP=1", 3000);
  }

  //Switch SMS "GSM" encoding.
  if (!CheckATReplyExists("AT+CSCS?", "+CSCS: \"GSM\"", 3000)) {
    ATCommand("AT+CSCS=\"GSM\"", 3000);
  }

  if (!CheckGSMRegisteredOK()) {
    //reboot now!
    if (debug)
    {
      Serial.println("Restarting GSM (reseting / rebooting A9G chip)...");
    }
    //Restart GSM (reset / reboot A9G chip):
    SoftResetA9();
  }
}

void SoftResetA9() {

  if (debug)
  {
    Serial.print("GSM: power cycle...");
  }
  digitalWrite(A9ResetPin, LOW);
  digitalWrite(A9LowPowerPin, LOW);
  digitalWrite(A9PowerPin, LOW);
  sleep(3000);
  digitalWrite(A9LowPowerPin, HIGH);
  digitalWrite(A9PowerPin, HIGH);
  if (debug)
  {
    Serial.println(" [ DONE ]");
  }
  //reboot
  TriggerWatchdogReset();
}

void SendSMS(String Phone, String Message) {
  A9SwitchOn();
  GSMInit();
  ATCommand("AT+CMGS=\"" + Phone + "\"\r " + Message + "\n \x1a", 10000);
}

void setup()
{
  ESP.wdtDisable();
  ESP.wdtEnable(WatchdogTimeout);
  Serial.begin(BaudRate);
  gsmSerial.begin(A9BaudRate, SWSERIAL_8N1, A9RXPin, A9TXPin, false, 256);

  pinMode(A9PowerPin, OUTPUT);
  pinMode(A9ResetPin, OUTPUT);
  pinMode(A9LowPowerPin, OUTPUT);

  digitalWrite(A9ResetPin, HIGH);
  sleep(1000);
   digitalWrite(A9ResetPin, LOW);
  digitalWrite(A9LowPowerPin, HIGH);
  digitalWrite(A9PowerPin, HIGH);


  A9SwitchOn();
  GSMInit();
  //SendSMS(OwnerPhone, TestMessage);


  //ATCommand("AT+CGATT=1", 3000, DEBUG);
  //ATCommand("AT+CGDCONT=1,"IP","mnet"", 1000, DEBUG); //Put your APN instead of mnet
  //ATCommand("AT+CGACT=1,1", 3000, DEBUG);
  //ATCommand("AT+CSQ", 1000, DEBUG);

  //String cmdString = "AT+HTTPGET=\"YOUR URL\"";
  //ATCommand(cmdString, 7000, DEBUG);
  //cmdString = "AT+HTTPGET=\"YOUR URL\"";
  //ATCommand(cmdString, 5000, DEBUG);
}


void TriggerWatchdogReset() {
  if (debug)
  {
    Serial.print("Initiating hardware watchdog reboot...");
  }
  while (true) {
  }
}

void loop()
{
	//GetGPSLocation();
  while (gsmSerial.available() > 0) {
    //RespondToCallOrSMS(IncomingMessage);
    Serial.write(gsmSerial.read());

    //do background esp8266 tasks
    yield();
    ESP.wdtFeed();
  }
  while (Serial.available() > 0) {
    gsmSerial.write(Serial.read());

    //do background esp8266 tasks
    yield();
    ESP.wdtFeed();
  }
}
