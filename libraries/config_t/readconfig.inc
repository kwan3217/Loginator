#include "Serial.h"
#include "Stringex.h"
#include <string.h>

template<class Pk, class Pr, class S> inline
bool ReadConfig<Pk,Pr,S>::begin() {
  //Config file is in the form:
  //0) Zero or more non-printables <34 (takes care of remainder of 0D0A line ending)
  //1) One or more non-space printables of a tag, or a # which makes the rest of the line (to nonprintable <32) a comment
  //2) One space
  //3) One or more characters of data, ended by a nonprintable <32 (intended to be 0A or 0D0A)
  //data structure is determined by the tag. The tag is not case
  //sensitive (converted to uppercase internally). Total config file size is
  //limited to 1 sector (512 bytes). Tags have a maximum of 10 characters.
  //This code doesn't have to be particularly fast, since it is called once before
  //the run. It doesn't need to conserve stack too much since nothing else is
  //running while this is. Prefer using stack for temporaries, so memory is
  //reclaimed and not statically allocated.
  char buf[512]; //Put this on the stack, we are called early in the processing
                 //before much stack is used, and don't want to hold it
                 //permanently.
  File<Pk,Pr,S> f(fs);

  if(!f.openr(configFilename)) FAIL(100*f.errno+21);
  int size=f.size()>512?512:f.size();
  if(!f.read(buf)) FAIL(100*f.errno+22);
  //Dump the whole file into a packet, including its direntry. This will capture
  //the file name, date and size. The FileCircular buffer is big enough to
  //handle it if the calling function calls dump right after
  #include "write_packet_config.INC"
//  packet.fill(f.de.entry,sizeof(f.de.entry));
//  packet.fill(buf,size);
  void* tagData;
  int* arrSize;
  int tagType;
  char tag[11];  //Storage for the tag
  int tagp=0;
  int partindata=0;
  int datastart=0;
  for(int i=0;i<size;i++) {
    Serial.print(buf[i]);
    switch(partindata) {
      case 0: //Handle part 0, line part before tag
        if(buf[i]=='#') { //Found beginning of a comment
          partindata=4;
          break;
        } else if(buf[i]>=33) { //Found a printable character
          partindata=1;
          //Intentionally fall through so tag handler below gets to look at this char
        } else {
          break; //not a printable character, keep looking
        }
      case 1: //tag
        if(buf[i]!=' ') { //Found more tag characters
          if(buf[i]>='a' && buf[i] <='z') buf[i]-=32; //Force upper case
          tag[tagp]=buf[i];
          tagp++;
        } else { //Found the tag end
          partindata=3;
          datastart=i;
          //Finish this tag and reset for the next one
          tag[tagp]=0;
          tagp=0;
          bool found=false;
          for(int j=0;tagNames[j];j++) {
            if(strcmp(tag,tagNames[j])==0) {
              tagType=entries[j].tagType;
              tagData=entries[j].tagData;
              arrSize=entries[j].tagSize;
              found=true;
            }
          }
          if(!found)FAIL(23);
        }
        break;
      case 3: //data
        if(buf[i]<32||buf[i]=='#') {
          //Found the end of the data
          buf[i]=0;
          partindata=buf[i]=='#'?4:0; //If it's a comment, skip the rest of the line
          if(!handleData(buf+datastart,tagData,arrSize,tagType)) return false; //handleData will set errno if necessary
        }
        break;
      case 4: //comment
        if(buf[i]<32) {
          //Found the end of the line
          partindata=0;
        }
        break;
    }
  }
  if(partindata==3) if(!handleData(buf+datastart,tagData,arrSize,tagType)) return false; //handleData will set errno if necessary
  return true;
}

template <class Pk, class Pr, class S> inline
bool ReadConfig<Pk,Pr,S>::handleData(char* buf,void* tagData,int* arrSize, int tagType) {
  if(tagType==typeInt) {
    int* tagInt=static_cast<int*>(tagData);
    if(arrSize) {
      return handleIntArr(buf,tagInt,arrSize);
    } else {
      return handleInt(buf,tagInt);
    }
  } else if(tagType==typeFp) {
    fp* tagFp=static_cast<fp*>(tagData);
    if(arrSize) {
      return handleFpArr(buf,tagFp,arrSize);
    } else {
      return handleFp(buf,tagFp);
    }
  } else if(tagType==typeV2) {
    Vector<2>* tagV2=static_cast<Vector<2>*>(tagData);
    if(arrSize) {
      return handleVector2Arr(buf,tagV2,arrSize);
    } else {
      return handleVector(buf,tagV2,2);
    }
  } else if(tagType==typeV3) {
    //Yes, we are intentionally breaking array bounds
    Vector<2>* tagV2=static_cast<Vector<2>*>(tagData);
    if(arrSize) {
      FAIL(27); //We don't handle this case (don't need it yet)
    } else {
      return handleVector(buf,tagV2,3);
    }
  } else {
    FAIL(24);
  }
  return true;
}

template <class Pk, class Pr, class S> inline
bool ReadConfig<Pk,Pr,S>::handleInt(char* buf,int* tagInt) {
  trim(buf);
  *tagInt=stoi(buf);
  return true;
}

template <class Pk, class Pr, class S> inline
bool ReadConfig<Pk,Pr,S>::handleFp(char* buf, fp* tagFp) {
  trim(buf);
  *tagFp=stof(buf);
  return true;
}

template <class Pk, class Pr, class S> inline
bool ReadConfig<Pk,Pr,S>::handleIntArr(char* buf, int* tagInt, int* arrSize) {
  trim(buf);
  *arrSize=0;
  int n=strlen(buf);
  int bp=0;
  for(int i=0;i<n;i++) {
    if(buf[i]==' ') {
      buf[i]=0;
      tagInt[*arrSize]=stoi(buf+bp);
      bp=i+1;
      (*arrSize)++;
    }
  }
  tagInt[*arrSize]=stoi(buf+bp);
  (*arrSize)++;
  return true;
}

template <class Pk, class Pr, class S> inline
bool ReadConfig<Pk,Pr,S>::handleFpArr(char* buf, fp* tagFp, int* arrSize) {
  trim(buf);
  *arrSize=0;
  int n=strlen(buf);
  int bp=0;
  for(int i=0;i<n;i++) {
    if(buf[i]==' ') {
      buf[i]=0;
      tagFp[*arrSize]=stof(buf+bp);
      bp=i+1;
      (*arrSize)++;
    }
  }
  tagFp[*arrSize]=stof(buf+bp);
  (*arrSize)++;
  return true;
}

template <class Pk, class Pr, class S> inline
bool ReadConfig<Pk,Pr,S>::handleVector(char* buf, Vector<2>* tagV, int n_comp) {
  trim(buf);
  int i_comp=0;
  int n=strlen(buf);
  int bp=0;
  for(int i=0;i<n;i++) {
    if(buf[i]==' ') {
      buf[i]=0;
      (*tagV)[i_comp]=stof(buf+bp);
      bp=i+1;
      i_comp++;
    }
  }
  (*tagV)[i_comp]=stof(buf+bp);
  i_comp++;
  if(i_comp!=n_comp) FAIL(25); //wrong number of components
  return true;
}

template <class Pk, class Pr, class S> inline
bool ReadConfig<Pk,Pr,S>::handleVector2Arr(char* buf, Vector<2>* tagV2, int* arrSize) {
  trim(buf);
  *arrSize=0;
  int n=strlen(buf);
  int bp=0;
  int i_comp=0;
  for(int i=0;i<n;i++) {
    if(buf[i]==' ') {
      buf[i]=0;
      tagV2[*arrSize][i_comp]=stof(buf+bp);
      bp=i+1;
      i_comp=(i_comp+1)%2;
      if(i_comp==0) (*arrSize)++;
    }
  }
  tagV2[*arrSize][i_comp]=stof(buf+bp);
  i_comp=(i_comp+1)%2;
  if(i_comp==0) { //Did we read a whole vector?
    (*arrSize)++;
  } else {
    FAIL(26);
  }
  return true;
}

