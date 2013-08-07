#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

//Internet Stuff
byte mac[] = { 
  0x86, 0x8D, 0xBE, 0x8F, 0xFE, 0xED };
char server[] = "10.32.25.136";
IPAddress serverIP(10,32,25,136);
int sendPort = 4001;
int recvPort = 4002;

//My Database Values
int myId = -1;
char myName[] = "PetersDesk";
char idStr[10];

//Sensor Stuff
int sensorPin = A0;
int sensorValue = 0;
int lastSensorVal = 0;

//Clients
EthernetClient client;
EthernetUDP Udp;

//Strings
char contype[] = "Content-Type: application/x-www-form-urlencoded";
char conclose[] = "Connection: close";
char messageBuffer[10];

int debugInfoTCP = 0;
int debugInfoUDP = 0;
int moreDebugInfo = 0;
int delayAmount = 20;


void setup() {
  Serial.begin(9600);
  Serial.print(".");
  if(Ethernet.begin(mac)==0){
    Serial.println("Failed to configure Ethernet with DHCP");
    while(true);
  }
  else Serial.println("Connected to Ethernet");
  delay(1000);

  addMe();

  intToStr(myId,idStr);

  Serial.print("ID:");
  Serial.println(*idStr);

  Udp.begin(sendPort);
}



void loop()
{
  Serial.print(',');
  lastSensorVal = sensorValue;
  sensorValue = analogRead(sensorPin);
  int diff = lastSensorVal - sensorValue;
  if(diff>2 || diff<-2){
    Udp.beginPacket(serverIP, sendPort);
    Udp.write("set ");
    Udp.write(idStr);
    Udp.write(' ');
    Udp.write(intToStr(sensorValue,messageBuffer));
    Udp.endPacket();
    Serial.print("Sending ");
    Serial.println(sensorValue);
  }
  delay(100);
}

char *intToStr(int num, char *buffer){
  String str = "";
  str+=num;
  Serial.print("Str:");
  Serial.println(str);
  
  str.toCharArray(buffer,10);
  Serial.print("buf:");
  Serial.println(buffer);
  return buffer;
}

void addMe(){
  TCPConnect();

  client.println("POST /sensors/add HTTP/1.0");
  client.println(contype);
  client.print("Content-Length: ");
  client.println((sizeof(myName)/sizeof(myName[0]))+4);
  client.println(conclose);
  client.println();
  client.print("city=");
  client.println(myName);
  client.println();

  delay(50);

  while (myId == -1) {
    if(client.available()){
      char c = client.read();
      Serial.print(c);

      if(c=='i'){
        getId();
      }
    }
    else delay(100);
  }

  TCPStop();
}


//Connects to server via TCP
void TCPConnect(){
  if(debugInfoTCP)
    Serial.println("connecting TCP...");
  while(!client.connect(server,4000)){
    if(debugInfoTCP)
      Serial.println("Connection failed, trying again in 1s");
    delay(1000);
  }
  if(debugInfoTCP)
    Serial.println("connected TCP");
}

void TCPStop(){
  while(client.available())
    client.read();
  client.flush();
  if(!client.connected()){
    client.stop();
    Serial.println("Stopping TCP...");
  }
}

void getId(){
  char c = client.read();
  Serial.print(c);
  if(c=='d'){
    c = client.read();
    Serial.print(c);
    if(c==':'){
      c = client.read();
      Serial.print(c);
      if(c >= '0' && c<= '9'){
        myId=c-48;
        c = client.read();
        while(c>='0' && c<='9'){
          myId*=10;
          myId+=(c-48);
          c = client.read();
        }
        Serial.print(String("\nMyId found: ") + myId + '\n');
      }
    }
  }
}

