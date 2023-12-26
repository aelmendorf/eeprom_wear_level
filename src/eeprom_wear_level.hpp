/* .+

.context    : eeprom_wear_level EEPROM wear leveling library
.title      : eeprom_wear_level definition  
.kind       : c++ source
.author     : Andrew Elmendorf <aelmendorf234@gmail.com>
.site       : South Carolina - USA
.creation   : 24-Dec-2023
.copyright  : (c) 2023 Andrew Elmendorf
.license    : GNU Lesser General Public License

.description
    eeprom_wear_level extends EEPROM life by dsitributing data writes over
    a specified range

.warning
    startAddr=0 is not allowed
    blockNumber must be 3x the data size

.compile_options


.- */


#pragma once
#include <EEPROM.h>

#define WRITE_UPPER     254
#define WRITE_LOWER     1
#define WRITE_DEFAULT   5
#define ADDRESS_MIN     1


class eeprom_wear_level{
private:
    int blockMark=0xfe; 
    int blockSize;
    int blockNumber;
    int startAddr;
    int endAddr;
    int blockAddr=0;
    int countAddr=0;
    byte writeLimit;
    byte writeCount=0;
    
public:
    template<class T> eeprom_wear_level(T& data,int blockNum,int start,byte wLimit=1){
        this->startAddr=start<ADDRESS_MIN ? 1:start;
        this->blockSize=sizeof(data)+2;  //1byte for mark, 1byte for writeCount, xbytes for data

        this->blockNumber=(blockNum>=2) ? blockNum:2;
        this->startAddr=start;
        //fource write in range
        this->writeLimit=(wLimit<=WRITE_UPPER && this->writeLimit>=WRITE_LOWER) ? wLimit:WRITE_DEFAULT;
        this->endAddr=this->startAddr+this->blockSize+this->blockNumber;
    }

/**
 * Find block address if previous data exist
*/
    void begin(){
        this->blockAddr=0;
        this->countAddr=0;
        this->writeCount=0;
        for(int addr=this->startAddr;addr<this->endAddr;addr+=this->blockSize){
            if(EEPROM[addr]==this->blockMark){
                this->blockAddr=addr;
                this->countAddr=addr+1;
                this->writeCount=EEPROM[this->countAddr];
                break;
            }
        }
    }
/**
 * Get data if previous data exists
*/
    template<class T> int get(T& data){
        //return 0 if not previous data exists
        if(!this->blockAddr)
            return 0;
        
        //get pointer to write data into
        this->writeCount=EEPROM[this->countAddr];
        byte* p = (byte*)(void*)&data;
        for (int dataAddr = this->blockAddr+2; dataAddr<(this->blockAddr+this->blockSize);dataAddr++){
            *p++=EEPROM[dataAddr];
        }
        return 1;
    }
/**
 *  Writes data to eeprom.  Moves to new block if write limit is hit
*/
    template<class T> void put(const T& data){
        //if blockAddr==0 then is first write.  set to startAddr
        if(!this->blockAddr){
            this->blockAddr=this->startAddr;
            this->countAddr=this->blockAddr+1;
            this->writeCount=0;
        }
        //if write count >= limit then format old block then increment blockAddr
        if(this->writeCount>=this->writeLimit){
            int old_addr;
            old_addr=this->blockAddr;
            this->blockAddr+=this->blockSize;

            //If blockAddr>=endAddr then wrap around to startAddr
            if(this->blockAddr>=this->endAddr){
                this->blockAddr=this->startAddr;
            }
            //set countAddr and clear writeCount
            this->countAddr=this->blockAddr+1;
            this->writeCount=0;
            //format old block
            this->format_range(old_addr,old_addr+this->blockSize);
        }
    
        this->writeCount++;                             //increment writeCount
        EEPROM.put(this->blockAddr,this->blockMark);    //Write blockMarker
        EEPROM.put(this->countAddr,this->writeCount);   //Write writeCount

        //Write data
        const byte* p = (const byte*)(const void*)&data;
        for (int dataAddr = this->blockAddr+2; dataAddr < this->blockAddr+this->blockSize; dataAddr++) {
            EEPROM.update(dataAddr, *p++);
        }
    }
    //Format all blocks
    void format_all(){
        for(int addr=this->startAddr;addr<this->endAddr;addr++){
            EEPROM.update(addr,0xff);
        }
    }
    //format specified range
    void format_range(int start,int stop){
        for(int addr=start;addr<stop;addr++){
            EEPROM.update(addr,0xff);
        }
    }
    //print control variables
    void print_control(){
        Serial.print("BlockAddr: ");Serial.print(this->blockAddr,DEC);
        Serial.print(" CountAddr: ");Serial.print(this->countAddr,DEC);
        Serial.print(" WriteCount: ");Serial.print(this->writeCount,DEC);
        Serial.print(" BlockSize: ");Serial.print(this->blockSize,DEC);
        Serial.print(" StartAddr: ");Serial.print(this->startAddr,DEC);
        Serial.print(" EndAddr: ");Serial.println(this->endAddr,DEC);
    }
};