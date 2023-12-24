/* .+

.context    : eeprom_wear_level EEPROM wear leveling library
.title      : Track a simulated runtime log between power cycles.  
.kind       : c++ source
.author     : Andrew Elmendorf <aelmendorf234@gmail.com>
.site       : South Carolina - USA
.creation   : 24-Dec-2023
.copyright  : (c) 2023 Andrew Elmendorf
.license    : GNU Lesser General Public License

.description
  This application simulates tracing a elapsed time log bewteen power cycles.
  If this is the first time using the eeprom_wear_level library make sure to format
  the storage space by changing INITIALIZE_BLOCK to 1.  eeprom_wear_level operates by
  checking for a mark and the number of writes.  If the mark is found and the write count
  is below the set limit the data is written back to the same address.  If the mark is found
  and the write count is greater than the write limit the data is written to the next block.

  This application reads a data structure from EEPROM, manipulates it, then writes the data structure
  back to EEPROM.  This is done on a period og 5 seconds.  WARNING** this is not best practice, the program should hold the data
  in memory until and write to EEPROM at longer intervals.  How long the intervals are should be determined by how sensitve the 
  data being logged is.  In the conext of this example, the log is runtimes for some running test.  The interval should be determined 
  based on how sensitive the test is to missed time.
  
.- */

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