
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

char* MYSSID = "CMCC-AUTO-G";
char* MYPASSWORD = "12344321";

const char * MYHOST = "https://cjlu.online/ShaoBing/Fuck.php";
const int  MYHOSTPORT = 80;
//const char *keys[] = {"res","info"};//需要收集的响应头的信息
boolean isCommand = false;
boolean isConnectedFlag = false;
boolean dataWaitPOST = false;
boolean receiveOver = false;
char inByte;
String RxDataString;

boolean isWIFIConnected() {
  isConnectedFlag = !(WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_DISCONNECTED);
  Serial.print(isConnectedFlag);
  return isConnectedFlag;
}

boolean getIsWIFIConnected()
{
    isConnectedFlag =  WiFi.isConnected();
    return isConnectedFlag;
}
  
void wifiConnect(char* ssid, char* passwd) {
  WiFi.begin(ssid, passwd);
  Serial.printf("Connecting to %s\n", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("Connected to %s\n", ssid);
  Serial.println("[Connected]");
  isConnectedFlag = true;
}


String  getValue()
{
  String temp;
  while (true)
  {
        if (Serial.available() > 0)
        {
              inByte = Serial.read();
            
              if (inByte == '[')
              {
                isCommand = true;
                temp = "";
              }
              else if (inByte == ']')
              {
                isCommand = false;
                temp += '\0';
                //dataWaitPOST = true;
                receiveOver = true;
                //return temp;
              }
              else if (isCommand)
              {
                temp += inByte;
              }

              if(receiveOver)
              {
                receiveOver = false;
                  if(temp == "LinkStart")
                  {
                      if(getIsWIFIConnected())
                      {
                          Serial.println("[Connected]");
                        }
                        else
                        {
                          wifiConnect(MYSSID,MYPASSWORD);
                          }
                    
                    }
                  else if(temp == "BreakLink")
                  {
                      WiFi.disconnect(false);
                      Serial.println("[UnConnected]");
                    }
                    else
                    {
                       dataWaitPOST = true;
                        return temp;
                      }
                           
                   
              }
          
        }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  
  

}

void loop() {
   RxDataString = getValue();
   if(WiFi.isConnected() && dataWaitPOST == true)
   {
        HTTPClient http;
        WiFiClient client;
        Serial.print("HTTP begin...\n");
        bool httpResponse_BOOL = http.begin("http://47.103.115.138/ShaoBing/Fuck.php");
        if (httpResponse_BOOL) {  // HTTP
        
        http.setReuse(0);
        http.setTimeout(1000);
        http.setUserAgent("Your G");
        http.setAuthorization("esp8266","boge");//用户校验信息
        http.addHeader("G","firestaradmin");
        
        int responseCode = http.POST(RxDataString);
        delay(200);
        Serial.println(responseCode);
        Serial.println(http.getString());
        dataWaitPOST = 0;
      }
      else if(!httpResponse_BOOL)
      {
          Serial.println("[HttpFailed]");
      }
      
      //delay(2000);
  }
  else
  {
    isConnectedFlag = false;
    Serial.println("[UnConnected]");
    //wifiConnect(MYSSID,MYPASSWORD);     
  }
}
