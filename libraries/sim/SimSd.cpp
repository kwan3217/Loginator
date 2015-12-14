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
  if(value==0) {
    ::printf("Card selected\n");
    cmdptr=0;
  } else {
    ::printf("Card deselected\n");
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

uint32_t SimSd::read_S0SPDR() {
  if(!dataReady ||dataDelayBytes>0) ::printf("S0SPDR read: ");
  if(dataReceive) {
    return 0xFF;
  } else if(responseReady) {
    if(responseDelayBytes>0) {
      ::printf("Thinking things over for %d more SPI cycles. Have an 0xFF while you wait.\n",responseDelayBytes);
      responseDelayBytes--;
      return 0xFF;
    }
    uint8_t result=response[responsePtr];
    ::printf("Returning response byte %d, value=0x%02x (%d)\n",responsePtr,result,result);
    responsePtr++;
    if(responsePtr==responseLen) {
      responseReady=false;
      ::printf("That's the end of the response\n");
    }
    return result;
  } else if(dataReady) {
    if(dataDelayBytes>0) {
      ::printf("Thinking things over for %d more SPI cycles. Have an 0xFE while you wait.\n",dataDelayBytes);
      dataDelayBytes--;
      return 0xFE;
    }
    uint8_t result=data[dataPtr];
    if(0==(dataPtr%16)) ::printf("%04x  ",dataPtr);
    ::printf("%02x",result);
    dataPtr++;
    if(0==(dataPtr%16)) {
      printf("  ");
      for(int j=0;j<16;j++) ::printf("%c",data[dataPtr-16+j]>=32&&data[dataPtr-16+j]<127?data[dataPtr-16+j]:'.');
      ::printf("\n"); 
    } else if(0==(dataPtr% 4)) ::printf(" "); 
    if(dataPtr==dataLen) {
      dataReady=false;
      ::printf("\nThat's the end of the data\n");
    }
    return result;
  } else {
    ::printf("SD card has nothing to say to you. value=0xFF\n");
    return 0xFF;
  }
}

void SimSd::write_S0SPDR(uint32_t value) {
  if(!dataReady || dataDelayBytes>0) ::printf("S0SPDR written, value=0x%02x (%d)\n",value,value);
  if(dataReceive) {
    if(dataReady) {
      data[dataPtr]=value;
      if(0==(dataPtr%16)) ::printf("%04x  ",dataPtr);
      ::printf("%02x",uint8_t(data[dataPtr]));
      dataPtr++;
      if(0==(dataPtr%16)) {
        printf("  ");
        for(int j=0;j<16;j++) ::printf("%c",data[dataPtr-16+j]>=32&&data[dataPtr-16+j]<127?data[dataPtr-16+j]:'.');
        ::printf("\n"); 
      } else if(0==(dataPtr% 4)) ::printf(" "); 
      if(dataPtr==dataLen) {
        dataReceive=false;
        fwrite(data,1,512,card);
      }
    } else {
      dataReady=(0xFE==value);
    }
  } else if(cmdptr<6) {
    if(cmdptr==0) {
      if(value & 0x80) {
        if(!dataReady || dataDelayBytes>0) ::printf("SD card, Wake up!\n");
      } else {
        ::printf("Receiving command byte 0x%02x\n",value & 0xFF);
        cmd=value & 0x3f;
        cmdptr++;
      }
    } else if(cmdptr<5) {
      ::printf("Receiving argument byte %d=0x%02x\n",cmdptr,value & 0xFF);
      //Receive argument, MSB first
      arg=arg<<8 | (value & 0xFF);
      cmdptr++;
    } else if(cmdptr==5) {
      ::printf("Receiving CRC byte 0x%02x\n",value & 0xFF);
      crc=(value & 0xFE) >> 1;
      ::printf("Received complete command 0x%02x 0x%08x 0x%02x\n",cmd,arg,crc);
      cmdptr=0;
      executeCommand();
    }
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
  responsePtr=0;
  responseReady=true;
  responseDelayBytes=2;
  if(nextCommandAppSpecific) ::printf("A");
  ::printf("CMD%02d: ",cmd);
  int result;
  switch(cmd) {
    case CMD_GO_IDLE_STATE: 
      ::printf("Go Idle\n");
      responseLen=1;
      response[0]=R1_IDLE_STATE; //Low bit set, indicating card is idle
      nextCommandAppSpecific=false;
      break;
    case CMD_APP: 
      ::printf("Next command is application-specific\n");
      responseLen=1;
      nextCommandAppSpecific=true;
      response[0]=R1_IDLE_STATE; //Low bit set, indicating card is idle
      break;
    case CMD_SD_SEND_OP_COND:
      if(!nextCommandAppSpecific) {
        ::printf("This is supposed to be an app specific command, I didn't get a CMD_APP before this. Executing it anyway, since I know what you mean...");
      }
      ::printf("SD Send OP Condition\n");
      responseLen=1;
      response[0]=0;
      nextCommandAppSpecific=false;
      HCS=(arg>>31) & 0x01;
      ::printf("Host sent me HCS=%d\n",HCS);
      break;
    case CMD_READ_OCR:
      ::printf("Read OCR\n");
      response[0]=0;
      response[1]=(OCR>>24) & 0xFF;                   
      response[2]=(OCR>>16) & 0xFF;                   
      response[3]=(OCR>> 8) & 0xFF;                   
      response[4]=(OCR>> 0) & 0xFF;
      nextCommandAppSpecific=false;
      responseLen=5;                   
      break;
    case CMD_SEND_IF_COND:
      ::printf("Send interface condition\n");
      responseLen=5;
      response[0]=R1_IDLE_STATE; //Low bit set, indicating card is idle
      response[1]=0; //Reserved
      response[2]=0; //Reserved
      response[3]=1; //2.7-3.6 volts acceptable
      response[4]=arg & 0xFF; //echo back the test pattern bits
      nextCommandAppSpecific=false;
      break;
    case CMD_SET_BLOCKLEN:
      ::printf("Set block length to %d\n",arg);
      response[0]=((isSDHC==1 && arg!=512)||(arg>512))?R1_ILL_COMMAND:0;
      if(response[0]==0) blocklen=arg;
      nextCommandAppSpecific=false;
      break;
    case CMD_SEND_CID:
      ::printf("Send Card ID register\n");
      nextCommandAppSpecific=false;
      responseLen=1;
      response[0]=0;
      dataLen=16;
      dataPtr=0;
      dataReady=true;
      dataDelayBytes=1;
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
      break;
    case CMD_SEND_CSD:
      ::printf("Send Card ID register\n");
      nextCommandAppSpecific=false;
      responseLen=1;
      response[0]=0;
      dataLen=16;
      dataPtr=0;
      dataReady=true;
      dataDelayBytes=1;
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
      break;
    case CMD_READ_SINGLE_BLOCK:
      ::printf("Read single block %u\n",arg);
      response[0]=R1_ILL_COMMAND;
      responseLen=1;
      ::printf("Seeking to %lu\n",size_t(arg)*size_t(512));
      if(fseek(card,size_t(arg)*size_t(512),SEEK_SET)!=0) break;
      result=fread(data,1,512,card);
      if(result!=512) break;
      response[0]=0;
      dataReady=true;
      dataDelayBytes=1;
      dataPtr=0;
      dataLen=512;
      break;
    case CMD_WRITE_SINGLE_BLOCK:
      ::printf("Write single block %u\n",arg);
      response[0]=R1_ILL_COMMAND;
      responseLen=1;
      ::printf("Seeking to %lu\n",size_t(arg)*size_t(512));
      if(fseek(card,size_t(arg)*size_t(512),SEEK_SET)!=0) break;
      if(result!=512) break;
      response[0]=0;
      dataPtr=0;
      dataLen=512;
      dataReceive=true;
      dataReady=false;
      break;
    default:
      nextCommandAppSpecific=false;
      responseLen=1;
      response[0]=R1_IDLE_STATE | R1_ILL_COMMAND;
      ::printf("Unrecognized command\n");
  }
}

//uint32_t SimSd::read_S0SPSR() {
//  if(!dataReady || dataDelayBytes>0) return SimSpi::read_S0SPSR();
//  return S0SPSR;
//}

const SimCid SimSd::cid={'K',{'W','A'},{'S','I','M','C','D'},0,3217,6,15};