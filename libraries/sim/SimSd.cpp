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

void SimSd::pinOut(int port, int pin, int value) {
  ::printf("SimSd::pinOut(port=%d, pin=%d, value=%d)\n",port,pin,value);
  cs=(0==value);
  ::printf(cs?"Card selected\n":"Card deselected\n");
  if(!cs) {
    state=WAIT_CMD;
  }
}

int SimSd::pinIn(int port, int pin) {
  int value=1; //If we are being read, then we have to return 1. This seems to represent a pullup on the CS line.
  ::printf("SimSd::pinIn(port=%d, pin=%d)=%d\n",port,pin,value);
  return value;
}

void SimSd::pinMode(int port, int pin, bool out) {
  ::printf("SimSd::pinMode(port=%d, pin=%d, dir=%s)\n",port,pin,out?"out":"in");
}

void SimSd::write_S0SPDR(uint32_t value) {
  SimSpi::write_S0SPDR(value); //Handle the SPIF flag
  switch(state) {
    case WAIT_CMD:
      S0SPDR=0xFF; //Nothing to say to the host
      if(0==(value & 0x80)) {
        cmd=value & 0x3F;
        argCount=4;
        state=WAIT_ARG;
      }
      break;
    case WAIT_ARG:
      S0SPDR=0xFF; //Nothing to say to the host
      arg=arg<<8 | (value & 0xFF);
      argCount--;
      if(argCount==0) state=WAIT_CRC;
      break;
    case WAIT_CRC:
      S0SPDR=0xFF; //Nothing to say to the host
      crc=(value & 0xFE) >> 1;
      ::printf("Received complete command 0x%02x 0x%08x 0x%02x\n",cmd,arg,crc);
      executeCommand();
      break;
    case RESPOND_START:
      S0SPDR=0xFF; //Delaying
      if(responseDelay>1) {
        responseDelay--;
      } else {
        state=RESPOND_SEND;
      }
      break;
    case RESPOND_SEND:
      S0SPDR=response[responsePtr]; //Delaying
      if(responseCount>1) {
        responsePtr++;
        responseCount--;
      } else if(readCount>0) {
        state=READ_START;
      } else if(writeCount>0) {
        state=WRITE_START;
      } else {
        state=WAIT_CMD;
      }
      break;
    case READ_START:
      S0SPDR=0xFE; //Delaying
      if(readDelay>1) {
        readDelay--;
      } else {
        state=READ;
      }
      break;
    case READ:
      S0SPDR=data[readPtr];
      if(readCount>1) {
        readPtr++;
        readCount--;
      } else {
        state=WAIT_CMD;
      }
      break;
    case WRITE_START:
      S0SPDR=0xFF; //Waiting for start byte
      if(0xFE==(value & 0xFF)) {
        state=WRITE;
      }
      break;
    case WRITE:
      data[writePtr]=value & 0xFF; 
      S0SPDR=0xFF;
      if(writeCount>1) {
        writePtr++;
        writeCount--;
      } else {
        fwrite(data,1,512,card);
        fflush(card);
        state=WAIT_CMD;
      }
      break;
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
  if(nextCommandAppSpecific) ::printf("A");
  ::printf("CMD%02d: ",cmd);
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
      ::printf("Go Idle\n");
      responseCount=1;
      response[0]=R1_IDLE_STATE; //Low bit set, indicating card is idle
      nextCommandAppSpecific=false;
      state=RESPOND_START;
      break;
    case CMD_APP: 
      ::printf("Next command is application-specific\n");
      responseCount=1;
      nextCommandAppSpecific=true;
      response[0]=R1_IDLE_STATE; //Low bit set, indicating card is idle
      state=RESPOND_START;
      break;
    case CMD_SD_SEND_OP_COND:
      if(!nextCommandAppSpecific) {
        ::printf("This is supposed to be an app specific command, but no CMD_APP before this. Executing it anyway, command is unambiguous...");
      }
      ::printf("SD Send OP Condition\n");
      responseCount=1;
      response[0]=0;
      nextCommandAppSpecific=false;
      HCS=(arg>>31) & 0x01;
      state=RESPOND_START;
      break;
    case CMD_READ_OCR:
      ::printf("Read OCR\n");
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
      ::printf("Send interface condition\n");
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
      ::printf("Set block length to %d\n",arg);
      responseCount=1;
      response[0]=((isSDHC==1 && arg!=512)||(arg>512))?R1_PARAM_ERR:0;
      if(response[0]==0) blocklen=arg;
      nextCommandAppSpecific=false;
      state=RESPOND_START;
      break;
    case CMD_SEND_CID:
      ::printf("Send Card ID register\n");
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
      ::printf("Send Card ID register\n");
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
      ::printf("Read single block %u\n",arg);
      //Set up the following so that if the (host file) read fails,
      //we report an error and don't send any data.
      response[0]=R1_PARAM_ERR;
      responseCount=1;
      readCount=0;
      state=RESPOND_START;

      ::printf("Seeking to %lu\n",size_t(arg)*size_t(512));
      if(fseek(card,size_t(arg)*size_t(512),SEEK_SET)!=0) break;
      result=fread(data,1,512,card);
      if(result!=512) break;

      //Now that the read worked, set the state to show that.
      readCount=514; //Include the CRC bytes also
      response[0]=0; 
      break;
    case CMD_WRITE_SINGLE_BLOCK:
      ::printf("Write single block %u\n",arg);
      //Set up the following so that if the (host file) seek fails,
      //we report an error and don't receive any data
      response[0]=R1_PARAM_ERR;
      responseCount=1;
      state=RESPOND_START;
      writeCount=0;

      ::printf("Seeking to %lu\n",size_t(arg)*size_t(512));
      if(fseek(card,size_t(arg)*size_t(512),SEEK_SET)!=0) break;

      //Now that the seek worked, set the state to show that.
      response[0]=0;
      writeCount=512;
      break;
    default:
      nextCommandAppSpecific=false;
      responseCount=1;
      response[0]=R1_IDLE_STATE | R1_ILL_COMMAND;
      ::printf("Unrecognized command\n");
      state=RESPOND_START;
  }
}

//uint32_t SimSd::read_S0SPSR() {
//  if(!dataReady || dataDelayBytes>0) return SimSpi::read_S0SPSR();
//  return S0SPSR;
//}

const SimCid SimSd::cid={'K',{'W','A'},{'S','I','M','C','D'},0,3217,6,15};
