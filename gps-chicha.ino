//ESP8266 A12S A9/A9G with ESPSoftwareSerial messaging via Interrupts, Hardware ESP8266  Watchdog restart.

#include <SoftwareSerial.h>

SoftwareSerial gsmSerial;

String OwnerPhone = "+79991234567";
//String Password = "34859475";
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
  if (message.indexOf("OK") >= 0)
  {
    if (debug)
    {
      Serial.println("A9/A9G is awailable and operational.");
    }
    GSMOperational = true;
  } else {
    if (debug)
    {
      Serial.println("A9/A9G is NOT awailable and NOT operational.");
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
      if (message.indexOf("OK") >= 0)
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
void GSMInit() {
  if (!CheckATReplyExists("AT+IPR?", String(A9BaudRate), 3000)) {
    ATCommand("AT+IPR=" + String(A9BaudRate) + "\"", 3000);
  } else {
    ATCommand("AT+CMGF=1", 3000);
  }

  if (CheckATReplyExists("AT+CREG?", "+CREG: 0", 3000)) {
    //+CREG: 0 (not registered and not searching)
    //reboot now!
    if (debug)
    {
      Serial.println("+CREG: 0 (not registered and not searching), Restart GSM (reseting / rebooting A9G chip)...");
    }
    //Restart GSM (reset / reboot A9G chip):
    SoftResetA9();
  } else {
    if (debug)
    {
      Serial.println("//normal - wait until receive  +CREG: 1.1 or +CREG: 6,1 or ...");
    }
  }

}

void SoftResetA9() {
  digitalWrite(A9ResetPin, HIGH);
  sleep(1000);
  digitalWrite(A9ResetPin, LOW);
}

void SendSMS(String Phone, String Message) {
  A9SwitchOn();
  GSMInit();
  ATCommand("AT+CMGS=\"" + Phone + "\"\r " + Message + "\n \x1a", 10000);
}

void setup()
{
  ESP.wdtDisable();
  //ESP.wdtEnable(WDTO_8S);
  ESP.wdtEnable(WatchdogTimeout);
  Serial.begin(BaudRate);
  gsmSerial.begin(A9BaudRate, SWSERIAL_8N1, A9RXPin, A9TXPin, false, 256);

  pinMode(A9PowerPin, OUTPUT);
  pinMode(A9ResetPin, OUTPUT);
  pinMode(A9LowPowerPin, OUTPUT);

  digitalWrite(A9ResetPin, LOW);
  digitalWrite(A9LowPowerPin, HIGH);
  digitalWrite(A9PowerPin, LOW);

  SendSMS(OwnerPhone, TestMessage);
  //ATCommand("AT+CMGS=\"+79288195014\"\r Test from at command\n \x1a", 5000, debug);
  //sleep(5000);


  //ATCommand("AT+CGATT=1", 3000, DEBUG);
  //ATCommand("AT+CGDCONT=1,"IP","mnet"", 1000, DEBUG); //Put your APN instead of mnet
  //ATCommand("AT+CGACT=1,1", 3000, DEBUG);
  //ATCommand("AT+CSQ", 1000, DEBUG);

  //String cmdString = "AT+HTTPGET=\"YOUR URL\"";
  //ATCommand(cmdString, 7000, DEBUG);
  //cmdString = "AT+HTTPGET=\"YOUR URL\"";
  //ATCommand(cmdString, 5000, DEBUG);
}


void loop()
{
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
