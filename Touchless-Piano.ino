// this example will play a track and then 
// every five seconds play another track
//
// https://archive.org/details/SynthesizedPianoNotes
// it expects the sd card to contain these three mp3 files
// but doesn't care whats in them
//
// sd:/mp3/0001.mp3
// sd:/mp3/0002.mp3
// sd:/mp3/0003.mp3
// ToDo: Add Random function, Add windchime function, Add webpage to select function. explore Blutooth source

#include <SoftwareSerial.h>
#include <DFMiniMp3.h>
//DFplayer VCC 5V
bool isPlaying=false;
 const byte pinDfpBusy = D3;

#include "Adafruit_VL53L0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
// D1 SCL ---- SCL VL530X
// D2 SDA ---- SDA VL530X
//VCC 3.3V
int tofRange=0;
uint16_t trackPlaying=0;

// implement a notification class,
// its member methods will get called 
//
class Mp3Notify
{
public:
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }

  static void OnPlayFinished(uint16_t globalTrack)
  {
    Serial.println();
    Serial.print("Play finished for #");
    Serial.println(globalTrack);
    isPlaying=false;
  }

  static void OnCardOnline(uint16_t code)
  {
    Serial.println();
    Serial.print("Card online ");
    Serial.println(code);     
  }

  static void OnCardInserted(uint16_t code)
  {
    Serial.println();
    Serial.print("Card inserted ");
    Serial.println(code); 
  }

  static void OnCardRemoved(uint16_t code)
  {
    Serial.println();
    Serial.print("Card removed ");
    Serial.println(code);  
  }
};

// instance a DFMiniMp3 object, 
// defined with the above notification class and the hardware serial class
//
//DFMiniMp3<HardwareSerial, Mp3Notify> mp3(Serial1);

// Some arduino boards only have one hardware serial port, so a software serial port is needed instead.
// comment out the above definition and uncomment these lines
SoftwareSerial secondarySerial(D5, D6); // RX, TX
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);

void setup() 
{
  Serial.begin(115200);

  Serial.println("initializing...");
  pinMode(pinDfpBusy, INPUT);   // init Busy pin from DFPlayer (lo: file is playing / hi: no file playing)  
  mp3.begin();

  uint16_t volume = mp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  mp3.setVolume(30);
  
  uint16_t count = mp3.getTotalTrackCount();
  Serial.print("files ");
  Serial.println(count);
  
  Serial.println("starting...");

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
 //   while(1);
  }
  
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // calling mp3.loop() periodically allows for notifications 
    // to be handled without interrupts
    mp3.loop(); 
    delay(1);
  }
}

void loop() 
{

  trackPlaying = mp3.getCurrentTrack();
  Serial.print("Current Track ");
  Serial.println(trackPlaying);

 

checkTOFrange();
if (tofRange > 0) { delay(10); checkTOFrange();} // check again to remove false values. remove this line to get random notes like windchime
//Serial.print("isPlaying ");
//Serial.println(isPlaying);
//DFPlayer
if ((digitalRead(pinDfpBusy) == HIGH)&&(tofRange>200 && tofRange<=1000)) {      // if no mp3 is playing -> play next file 
    Serial.print("track ");
    Serial.println((tofRange-175)/25);
    Serial.print("tof: ");
    Serial.println(tofRange);
    mp3.playMp3FolderTrack((tofRange-175)/25);  // sd:/mp3/0001.mp3
    isPlaying=true;
    delay(500);
}

    tofRange=0;
    delay(100);
  
}




void checkTOFrange(){
   VL53L0X_RangingMeasurementData_t measure;
    
  //Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("Distance (mm): "); 
    tofRange=measure.RangeMilliMeter;    
    Serial.println(tofRange);

  } else {
    Serial.println(" out of range ");
    tofRange=0;
  }
}
