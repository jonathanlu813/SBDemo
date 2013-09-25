/*
 SenseBox Demo
 
 This sketch connects to SenseCloud and makes a request
 using a Wiznet Ethernet shield. You can use the Arduino Ethernet shield, or
 the Adafruit Ethernet shield, either one will work, as long as it's got
 a Wiznet Ethernet module on board. This sketch also use a DHT11 temperature
 sensor.
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * DHT11 attached to pin A0
 
 created 06 Dec 2012
 by Jonathan Lu
 
 */

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 1, 130}; // a valid IP on your LAN

byte server[] =  {183, 179, 85, 119}; // SenseBox server IP
EthernetClient client;

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = (long)5*1000;  // delay between updates, in milliseconds

#define dht11_pin 14 //Analog port 0 on Arduino Uno

byte dht11_dat[5];  
byte dht11_in;

void setup()
{
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  pinMode(dht11_pin, OUTPUT);
  digitalWrite(dht11_pin, HIGH);
}

void loop(){
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    httpRequest();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void httpRequest() {
// send data to SenseCloud
   if (client.connect(server, 5000)) {
     dht11_read();
     
    Serial.println("connecting...");
    // Compose your POST body here
    String data = "value={\"temperature\":";
    data += dht11_dat[2];
    data += "}&tags[]=test&tags[]=test 2";
    Serial.println(data);
    
    client.println("POST /api/datastreams HTTP/1.1");
    client.println("X-Device-Key: baca9a14cc197ee89847e70521447ccf");
    client.println("X-App-Key: af0c90f2b3f8c92a215a790fc008284e");
    client.println( "Content-Type: application/x-www-form-urlencoded" );
    client.println( "Connection: close" );
    client.print( "Content-Length: " );
    client.println( data.length() );
    client.println();
    client.print( data );
    client.println();
    
    // note the time that the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}


// DHT11 - Part

byte read_dht11_dat()
{
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++)
  {
    while (!digitalRead(dht11_pin));
    delayMicroseconds(30);
    if (digitalRead(dht11_pin) != 0 )
      bitSet(result, 7-i);
    while (digitalRead(dht11_pin));
  }
  return result;
}
  
  
void dht11_read()   
{

  byte i;// start condition
       
  digitalWrite(dht11_pin, LOW);
  delay(18);
  digitalWrite(dht11_pin, HIGH);
  delayMicroseconds(1);
  pinMode(dht11_pin, INPUT);
  delayMicroseconds(40);    
    
  if (digitalRead(dht11_pin))
  {
    Serial.println("dht11 start condition 1 not met"); // wait for DHT response signal: LOW
    delay(1000);
    return;
  }
  delayMicroseconds(80);
  if (!digitalRead(dht11_pin))
  {
    Serial.println("dht11 start condition 2 not met");  //wair for second response signal:HIGH
    return;
  }
    
  delayMicroseconds(80);// now ready for data reception
  for (i=0; i<5; i++)
  {  dht11_dat[i] = read_dht11_dat();}  //recieved 40 bits data. Details are described in datasheet
    
  pinMode(dht11_pin, OUTPUT);
  digitalWrite(dht11_pin, HIGH);
  byte dht11_check_sum = dht11_dat[0]+dht11_dat[2];// check check_sum
  if(dht11_dat[4]!= dht11_check_sum)
  {
    Serial.println("DHT11 checksum error");
  }
}
