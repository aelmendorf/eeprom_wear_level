#include "eeprom_wear_level.hpp"


#define   LOG_PERIOD        5000   //5sec

#define   BLOCK_NUM         1024
#define   BLOCK_ADDR        128
#define   INITIALIZE_BLOCK  0 //1=first time,0=after initial data written.  


 typedef struct elap_log{
  unsigned long runtimes[7]={0,0,0,0,0,0,0};
  bool running=false;
  bool paused=false;
 }ElapsedLog;

ElapsedLog elapsed_log;
bool formatTriggered=false;
 unsigned long last_log_check;

//Setup eeprom_wear_level.  blockNum=1024,startAdr=128, and writeLimit=5
eeprom_wear_level wl(elapsed_log,BLOCK_NUM,BLOCK_ADDR,5);

void setup(){
  Serial.begin(38400);
  while(!Serial){  }
  //If first timer then format the block
  #if INITIALIZE_BLOCK
    Serial.println("----Formatting Storage Block----");
    wl.format_all();
    Serial.println("----Formatting Complete----");
  #endif
  //If you are not sure how much EEPROM you have use this
  //Serial.print("EEPROM Length: "); Serial.println(EEPROM.length(),DEC);
  //Begin must come before any read or writes.  format is an exception
  wl.begin();
  Serial.println("To format storage press F");
  Serial.println("----Initial Values----");
  read();//read and print data
  last_log_check=millis();
}


void loop(){
  unsigned long mill=millis();
  if(mill-last_log_check>=LOG_PERIOD){
    last_log_check=mill;
    if(formatTriggered){    //if F on serial then format and start over
      formatTriggered=false;
      Serial.println("----Formatting----");
      wl.format_all();
      wl.begin();
      read();
      Serial.println("----Formatting Complete----");
    }
    write();
    read();
    Serial.print("Control Values: ");
    wl.print_control();
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inByte = (byte)Serial.read();
    if(inByte=='F'){
      formatTriggered=true;
    }
  }
}

void read(){
  if(!wl.get(elapsed_log)){
      Serial.println("First Time.. Initializing..");
      for(int i=0;i<7;i++){
        elapsed_log.runtimes[i]=0;
      }
      elapsed_log.running=true;
      wl.put(elapsed_log);      
  }
  print_elapsed();
}

void write(){
  for(int i=0;i<7;i++){
    elapsed_log.runtimes[i]+=5;
  }
  elapsed_log.running=true;
  wl.put(elapsed_log);
}

void print_elapsed(){
  String msg=elapsed_log.running ? "true ":"false ";
  Serial.print("Elapsed Structure: Running: "+msg);Serial.print("Runtimes: ");
  for(int i=0;i<7;i++){
    Serial.print(" [");Serial.print(i);Serial.print("]=");
    Serial.print(elapsed_log.runtimes[i]);
  }
  Serial.println();
}