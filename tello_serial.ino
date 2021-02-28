/*******************************************
 * ESP8266 esp-01 serial client Tello Drone
 * 
 * Author: fvilmos
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>



/***********************************************
 * preprocessor directives, constants, variables
 ***********************************************/
#define DEBUG true                             /*if enabled debug winfrmation will be printed on serial*/

const String telloIP = "192.168.10.1";            /*tello IP*/
const String tello_SSID = "TELLO-601909";         /*name of the tello access point*/
const String tello_SSID_pass = "tello123";        /*name of the tello access point*/
const unsigned int cuiVideoPort = 11111;          /*video port*/
const unsigned int cuiCmdPort = 8889;             /*command port*/


char packetBuffer[UDP_TX_PACKET_MAX_SIZE];      /*buffer for received UDP packets*/
bool processString = false;                     /*indicats the end of string received on serial*/
String readString = "";                         /*holds the string recived from serial port*/


WiFiUDP UDPCommands;                          /* UDP object handler for commands*/
WiFiUDP UDPStream;                            /* UDP object handler for video stream*/

void setup()
{
  Serial.begin(115200);
  Serial.println();

  /*connect to Tello AP*/
  WiFi.begin(tello_SSID,tello_SSID_pass);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  /*set port to listen for UDPCommands / Video Stream*/
  UDPCommands.begin(cuiCmdPort);
  UDPStream.begin(cuiVideoPort);

  Serial.flush();

}


void loop() 
{

  char c = 0;                  /*holds the received characters from serial*/

  
  /*handle UDS messages received*/
  int packetSize = UDPCommands.parsePacket();
  
  if (packetSize)
  {
    /*erase message buffer*/
    memset(packetBuffer, 0, sizeof(packetBuffer));

    /*print statistics*/
    if (DEBUG)
    {
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, UDPCommands.remoteIP().toString().c_str(), UDPCommands.remotePort());
    }

    /*check received packets and print to console*/
    int len = UDPCommands.read(packetBuffer, packetSize);
    if (len > 0)
    {
      packetBuffer[len] = 0;
    }
    Serial.println(packetBuffer);
  }

  
  /*handle UDS messages received*/
  int packetSize1 = UDPStream.parsePacket();
  
  if (packetSize1)
  {
    /*erase message buffer*/
    memset(packetBuffer, 0, sizeof(packetBuffer));
    if (DEBUG)
    {
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize1, UDPStream.remoteIP().toString().c_str(), UDPStream.remotePort());
    }

    /*check received packets and print to console*/
    int len = UDPStream.read(packetBuffer, packetSize1);
    
    if (len > 0)
    {
      packetBuffer[len] = 0;
    }
    Serial.write(packetBuffer,len);
    
  }

  /*non blocking serial read*/
  if (Serial.available() > 0) 
  {
    c = Serial.read();
    
    if (c != '\n')
    {
      /*read characters till new lne received*/
      readString += c;
    }
    else
    {
      /*string ready to be processed*/
      processString = true;
    }
    
  } /*if*/

  /*command processor*/
  if(processString == true)
  {

    processString = false;

    /*put back to serial the received string*/
    if (DEBUG)
    {
      Serial.println(readString);
    }

    /*send directly the command*/
    UDPCommands.beginPacket(telloIP.c_str(), cuiCmdPort);
    UDPCommands.write(readString.c_str());
    UDPCommands.endPacket();


   /*clear string cotent*/ 
   readString = "";
   
  } /*if(processString == true)*/
    

  Serial.flush();

  /*recoonect on Wifi lost*/
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    //ESP.reset();
  }

    
} /*void loop() */
