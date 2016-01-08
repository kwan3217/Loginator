#ifndef readconfig_h
#define readconfig_h

#include "Quaternion.h"
#include "packet.h"
#include "file.h"
#include "cluster.h"

struct ReadConfigEntry {
  int   tagType;
  void* tagData;
  int*  tagSize;
};

/** Class to read a config file. Config files have the form

        tag1=value
        tag2=value #optional comment
        #optional comment
        tag3=value value value
        #etc

This class contains the code and tables necessary to parse a
config file of this form, and write the results wherever
needed. However, it does so by taking advantage of the ability
to split class code into multiple object files. We define all
the code in this object file, but we leave the definition of the
tables to the calling code. In the typical case, you will have
a number of ReadConfig:: tables in your main.cpp to finish off
this class. There can only be one config file reader per program
this way, but one is enough.
*/
template<class Pk, class Pr, class S>
class ReadConfig {
public:
  int errnum;
private:
  static const char configFilename[];
/** Define the tag names. This is a ragged array, IE an array of pointers
    to character (arrays). Since the array size is not determined at this
    point, you must include a nullptr as the last element of your array.
    Typically your code will have a definition of the following form:

        const char* const ReadConfig::tagNames[]={"TAG1","TAG2","TAG3",nullptr};

    The config file tags are not case sensitive, but this array must be
    in upper-case. Defined as an array of constant pointers to constant
    character (arrays) so that both the pointers and strings can be in
    read-only memory. */
  static const char* const tagNames[];
/** The type of each tag. In your code, put a definition similar to this:

        const int ReadConfig::tagTypes[]={tagInt,tagFp,tagIntArr};

    You don't have to have an entry corresponding to the last nullptr
    in tagNames (save 4 bytes this way). */
  static const ReadConfigEntry entries[];
/** Define pointers to the variables which will hold the values once read.
    In your code, put a definition similar to this:

        int tag1=3; //put in a default value in case this tag doesn't appear in the config file
        fp tag2=3.14159;
        const int maxEntries=10;
        int tag3[maxEntries];
        int n_entries;
	void *const ReadConfig::tagDatas[]={&tag1,&tag2,tag3};

    Any variable may hold config entries as long as it has a definite location
    in memory (so don't try to do it with function local variables). This
    includes member fields of an object.
*/
  static void *const tagDatas[];
/** Pointers to the variables which will hold the array lengths. If you
    ask for an int array or float array, there may be a varying number
    of array elements in the config file. The code will count these elements
    and populate the given variable with the number of elements. In your code,
    put a definition like this:

	void *const ReadConfig::tagSizes[]={nullptr,nullptr,&n_entries};
*/
  static int  *const tagSizes[];


private:
  Cluster<Pk,Pr,S>& fs;
  Pk& packet;
public:
  ReadConfig(Cluster<Pk,Pr,S>& Lfs, Pk& Lpacket):fs(Lfs),packet(Lpacket) {};
  bool begin();
  static const int typeInt=0;
  static const int typeFp=1;
  static const int typeV2=2;
  static const int typeV3=3;
private:
  bool handleData(char* buf, void* tagData, int* arrSize, int tagType);
  bool handleInt(char* buf, int* tagInt);
  bool handleFp(char* buf, fp* tagFp);
  bool handleVector(char* buf, Vector<2>* tagV2, int n);
  bool handleIntArr(char* buf, int* tagInt, int* arrSize);
  bool handleFpArr(char* buf, fp* tagFp, int* arrSize);
  bool handleVector2Arr(char* buf, Vector<2>* tagV, int* arrSize);
};

#include "readconfig.inc"

#endif
