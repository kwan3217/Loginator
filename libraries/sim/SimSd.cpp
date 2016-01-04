#include "SimSd.h" 
#include <sys/stat.h>

bool SimSd::open(char* cardfn) {
  card=fopen(cardfn,"rb+");
  struct stat sb;
  int fd=fileno(card);
  if(fd<0) return false;
  if(fstat(fd,&sb)<0) return false;
  
  csd.c_size = sb.st_size/(512*1024)-1;
  
  return(card!=0);
}

void SimSd::close() {
  fclose(card);
}

void SimSd::csOut(int value) {
  dprintf(SIMSD,"SimSd::csOut(value=%d)\n",value);
  cs=(0==value);
  dprintf(SIMSD,cs?"Card selected\n":"Card deselected\n");
  if(!cs) {
    state=WAIT_CMD;
  }
}

int SimSd::csIn() {
  int value=1; //If we are being read, then we have to return 1. This seems to represent a pullup on the CS line.
  dprintf(SIMSD,"SimSd::csIn()=%d\n",value);
  return value;
}

void SimSd::csMode(bool out) {
  dprintf(SIMSD,"SimSd::csMode(dir=%s)\n",out?"out":"in");
}

//In the real thing, the input and output are simultaneous, and the output
//cannot be calculated from the input. In the simulation, in order to enforce
//this, the sim code calls transferMISO (slave->master) first, then
//transferMOSI (master->slave). We have to be careful to only change state in
//one of these. Each branch of the case statement will be present in both
//transfers, but will be trivial in one of them. In particular, if a branch
//involves the input value, it must be nontrivial only in MOSI, and if a branch
//returns a non-idle result, it must be nontrivial only in MISO. If a branch
//only involves state-changes, we will arbitrarily put it in MOSI.
uint8_t SimSd::transferMISO() {
  uint8_t result; //Used when we have to calculate the return value, but do some other stuff which would change it before we return it
  switch(state) {
    case WAIT_CMD:
    case WAIT_ARG:
    case WAIT_CRC:
      return 0xFF; //Nothing to say to the host
    case RESPOND_START:
      return 0xFF; //Delaying
    case RESPOND_SEND:
      dprintf(SIMSD_TRANSFER,"In state RESPOND_SEND, responseCount=%d, readCount=%d, writeCount=%d\n",responseCount,readCount,writeCount);
      result=response[responsePtr];
      if(responseCount>1) {
        responsePtr++;
        responseCount--;
      } else if(readCount>0) {
        state=READ_START;
        dprintf(SIMSD_TRANSFER,"Going to state READ_START\n");
      } else if(writeCount>0) {
        state=WRITE_START;
        dprintf(SIMSD_TRANSFER,"Going to state WRITE_START\n");
      } else {
        state=WAIT_CMD;
        dprintf(SIMSD_TRANSFER,"Going to state WAIT_CMD\n");
      }
      return result;
    case READ_START:
      dprintf(SIMSD_TRANSFER,"In state READ_START, readDelay=%d\n",readDelay);
      if(readDelay>1) {
        readDelay--;
      } else {
        state=READ;
        dprintf(SIMSD_TRANSFER,"Going to state READ\n");
      }
      return 0xFE; //Send start condition
    case READ:
      dprintf(SIMSD_TRANSFER,"In state READ, readCount=%d\n",readCount);
      result=data[readPtr];
      if(readCount>1) {
        readPtr++;
        readCount--;
      } else {
        state=WAIT_CMD;
        dprintf(SIMSD_TRANSFER,"Going to state WAIT_CMD\n");
      }
      return result;
    case WRITE_START:
    case WRITE:
      return 0xFF;
  }
}

//The return value should not be a function of the input, since
//in the real thing, the two bytes pass each other on the line
//and the first bit of the response is sent before the last
//bit of the input is received.
void SimSd::transferMOSI(uint8_t value) {
  switch(state) {
    case WAIT_CMD:
      dprintf(SIMSD_TRANSFER,"In state WAIT_CMD\n");
      if(0==(value & 0x80)) {
        cmd=value & 0x3F;
        argCount=4;
        state=WAIT_ARG;
        dprintf(SIMSD_TRANSFER,"Going to state WAIT_ARG\n");
      }
      return;
    case WAIT_ARG:
      dprintf(SIMSD_TRANSFER,"In state WAIT_ARG, argCount=%d\n",argCount);
      arg=arg<<8 | (value & 0xFF);
      argCount--;
      if(argCount==0) {
    	state=WAIT_CRC;
        dprintf(SIMSD_TRANSFER,"Going to state WAIT_CRC\n");
      }
      return;
    case WAIT_CRC:
      crc=(value & 0xFE) >> 1;
      dprintf(SIMSD_TRANSFER,"Received complete command 0x%02x 0x%08x 0x%02x\n",cmd,arg,crc);
      executeCommand();
      return;
    case RESPOND_START:
      dprintf(SIMSD_TRANSFER,"In state RESPOND_START, %d delay left\n",responseDelay);
      if(responseDelay>1) {
        responseDelay--;
      } else {
        state=RESPOND_SEND;
        dprintf(SIMSD_TRANSFER,"Going to state RESPOND_SEND\n");
      }
      return;
    case RESPOND_SEND:
    case READ_START:
    case READ:
      return;
    case WRITE_START:
      dprintf(SIMSD_TRANSFER,"In state WRITE_START\n");
      if(0xFE==(value & 0xFF)) {
        state=WRITE;
        dprintf(SIMSD_TRANSFER,"Going to state WRITE\n");
      }
      return;
    case WRITE:
      dprintf(SIMSD_TRANSFER,"In state WRITE, writeCount=%d\n",writeCount);
      data[writePtr]=value & 0xFF;
      if(writeCount>1) {
        writePtr++;
        writeCount--;
      } else {
        dprintf(SIMSD_TRANSFER,"Writing data\n");
        fwrite(data,1,512,card);
        fflush(card);
        state=WAIT_CMD;
        dprintf(SIMSD_TRANSFER,"Going to state WAIT_CMD\n");
      }
      return;
  }
}

void write_bits(char data[], int datalen, uint32_t value, int hibit,int lobit) {
/*
  //Figure out how many bits there are in the value
  int vallen=hibit-lobit+1;
  //What part hangs over the highest byte?
  int hibyte=hibit/8;
  int lobyte=lobit/8;
  if(hibyte==lobyte) {
    //all in the same byte
    int bytenum=datalen-hibyte;
    int bitnum=lobyte
    */
}

void SimSd::executeCommand() {
  if(nextCommandAppSpecific) ::dprintf(SIMSD,"A");
  dprintf(SIMSD,"CMD%02d: ",cmd);
  responsePtr=0;
  responseDelay=2;
  readPtr=0;
  readDelay=1;
  readCount=0;
  writePtr=0;
  writeDelay=1;
  writeCount=0;
  int result;
  switch(cmd) {
    case CMD_GO_IDLE_STATE: 
      dnprintf("Go Idle\n");
      responseCount=1;
      response[0]=R1_IDLE_STATE; //Low bit set, indicating card is idle
      nextCommandAppSpecific=false;
      state=RESPOND_START;
      break;
    case CMD_APP: 
      dnprintf("Next command is application-specific\n");
      responseCount=1;
      nextCommandAppSpecific=true;
      response[0]=R1_IDLE_STATE; //Low bit set, indicating card is idle
      state=RESPOND_START;
      break;
    case CMD_SD_SEND_OP_COND:
      if(!nextCommandAppSpecific) {
    	dnprintf("This is supposed to be an app specific command, but no CMD_APP before this. Executing it anyway, command is unambiguous...");
      }
      dnprintf("SD Send OP Condition\n");
      responseCount=1;
      response[0]=0;
      nextCommandAppSpecific=false;
      HCS=(arg>>31) & 0x01;
      state=RESPOND_START;
      break;
    case CMD_READ_OCR:
      dnprintf("Read OCR\n");
      responseCount=5;
      response[0]=0;
      response[1]=(OCR>>24) & 0xFF;                   
      response[2]=(OCR>>16) & 0xFF;                   
      response[3]=(OCR>> 8) & 0xFF;                   
      response[4]=(OCR>> 0) & 0xFF;
      nextCommandAppSpecific=false;
      state=RESPOND_START;
      break;
    case CMD_SEND_IF_COND:
      dnprintf("Send interface condition\n");
      responseCount=5;
      response[0]=R1_IDLE_STATE; //Low bit set, indicating card is idle
      response[1]=0; //Reserved
      response[2]=0; //Reserved
      response[3]=1; //2.7-3.6 volts acceptable
      response[4]=arg & 0xFF; //echo back the test pattern bits
      nextCommandAppSpecific=false;
      state=RESPOND_START;
      break;
    case CMD_SET_BLOCKLEN:
      dnprintf("Set block length to %d\n",arg);
      responseCount=1;
      response[0]=((isSDHC==1 && arg!=512)||(arg>512))?R1_PARAM_ERR:0;
      if(response[0]==0) blocklen=arg;
      nextCommandAppSpecific=false;
      state=RESPOND_START;
      break;
    case CMD_SEND_CID:
      dnprintf("Send Card ID register\n");
      nextCommandAppSpecific=false;
      responseCount=1;
      response[0]=0;
      readCount=16;
      data[ 0]=cid.manufacturer;
      data[ 1]=cid.oem[0];
      data[ 2]=cid.oem[1];
      data[ 3]=cid.product[0];
      data[ 4]=cid.product[1];
      data[ 5]=cid.product[2];
      data[ 6]=cid.product[3];
      data[ 7]=cid.product[4];
      data[ 8]=cid.revision;
      data[ 9]=(cid.serial>>24) & 0xFF;
      data[10]=(cid.serial>>16) & 0xFF;
      data[11]=(cid.serial>> 8) & 0xFF;
      data[12]=(cid.serial>> 0) & 0xFF;
      data[13]=cid.manufacturing_month & 0x0F;
      data[14]=cid.manufacturing_year;
      data[15]=0xFF;
      state=RESPOND_START;
      break;
    case CMD_SEND_CSD:
      dnprintf("Send Card ID register\n");
      nextCommandAppSpecific=false;
      responseCount=1;
      response[0]=0;
      readCount=16;
      write_bits(data,16,csd.csd_structure     ,127,126);
      write_bits(data,16,csd.taac              ,119,112);
      write_bits(data,16,csd.nsac              ,111,104);
      write_bits(data,16,csd.tran_speed        ,103, 96);
      write_bits(data,16,csd.ccc               , 95, 84);
      write_bits(data,16,csd.read_bl_len       , 83, 80);
      write_bits(data,16,csd.read_bl_partial   , 79, 79);
      write_bits(data,16,csd.write_blk_misalign, 78, 78);
      write_bits(data,16,csd.read_blk_misalign , 77, 77);
      write_bits(data,16,csd.dsr_imp           , 76, 76);
      write_bits(data,16,csd.c_size            , 69, 48);
      write_bits(data,16,csd.erase_blk_en      , 46, 46);
      write_bits(data,16,csd.sector_size       , 45, 39);
      write_bits(data,16,csd.wp_grp_size       , 38, 32);
      write_bits(data,16,csd.wp_grp_enable     , 31, 31);
      write_bits(data,16,csd.r2w_factor        , 28, 26);
      write_bits(data,16,csd.write_bl_len      , 25, 22);
      write_bits(data,16,csd.write_bl_partial  , 21, 21);
      write_bits(data,16,csd.file_format_grp   , 15, 15);
      write_bits(data,16,csd.copy              , 14, 14);
      write_bits(data,16,csd.perm_write_protect, 13, 13);
      write_bits(data,16,csd.tmp_write_protect , 12, 12);
      write_bits(data,16,csd.file_format       , 11, 10);
      write_bits(data,16,    0xFF              ,  7,  0);
      state=RESPOND_START;
      break;
    case CMD_READ_SINGLE_BLOCK:
      dnprintf("Read single block %u\n",arg);
      //Set up the following so that if the (host file) read fails,
      //we report an error and don't send any data.
      response[0]=R1_PARAM_ERR;
      responseCount=1;
      readCount=0;
      state=RESPOND_START;

      dprintf(SIMSD,"Seeking to %lu\n",size_t(arg)*size_t(512));
      if(fseek(card,size_t(arg)*size_t(512),SEEK_SET)!=0) break;
      result=fread(data,1,512,card);
      if(result!=512) break;

      //Now that the read worked, set the state to show that.
      readCount=514; //Include the CRC bytes also
      response[0]=0; 
      break;
    case CMD_WRITE_SINGLE_BLOCK:
      dnprintf("Write single block %u\n",arg);
      //Set up the following so that if the (host file) seek fails,
      //we report an error and don't receive any data
      response[0]=R1_PARAM_ERR;
      responseCount=1;
      state=RESPOND_START;
      writeCount=0;

      dprintf(SIMSD,"Seeking to %lu\n",size_t(arg)*size_t(512));
      if(fseek(card,size_t(arg)*size_t(512),SEEK_SET)!=0) break;

      //Now that the seek worked, set the state to show that.
      response[0]=0;
      writeCount=512;
      break;
    default:
      nextCommandAppSpecific=false;
      responseCount=1;
      response[0]=R1_IDLE_STATE | R1_ILL_COMMAND;
      dnprintf("Unrecognized command\n");
      state=RESPOND_START;
  }
}

//uint32_t SimSd::read_S0SPSR() {
//  if(!dataReady || dataDelayBytes>0) return SimSpi::read_S0SPSR();
//  return S0SPSR;
//}

const SimCid SimSd::cid={'K',{'W','A'},{'S','I','M','C','D'},0,3217,6,15};
