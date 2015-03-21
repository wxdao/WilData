/*
 *WideSense Data File Form
 *WilData
 *version 0.2
 *Powered by libwaaaaxd v2
 */


#ifndef WILDATA_H
#define WILDATA_H

#include <cstddef>
#include <string>
#include <vector>


using namespace std;

typedef unsigned char byte;

class WilData{                                  //Base class,do not use
public:
  const char *getfilename();
  friend class WilData_W;
  friend class WilData_R;
  struct datainfo{
    long addr;
    long size;
    long dsize;
    byte hash[20];
  };
  const char* getmessage();
  vector<pair<string,datainfo>> getvdi();
  datainfo getdi(const char*);                  //get the datainfo by name
private:
  short ver=1;
  bool sign=false;
  const char *fn;
  string meg;
  vector<pair<string,datainfo>> vdi;
  vector<pair<byte*,long>> vd;
};

class WilData_W : public WilData{               //Writer
public:
  WilData_W(const char *filename);              //filename is the file you want to write
  WilData_W(){}

  bool open(const char *filename);              //see above
  void setmessage(const char*m);                //set the message in the archive
  void adddata(const char *name,byte *data,long size);//add data to archive(save in memory until write() is called)
  void addfile(const char *name,const char *filename);
  void write();                                 //write the data you added to the archive
};

class WilData_R : public WilData{
public:
  WilData_R(const char *filename);
  WilData_R(){}

  bool open(const char *filename);
  size_t getdata(const char *name,byte *data); //get data inside
  void appenddata(const char *name,byte *data,long size);//append data to archive(save in memory until write() is called)
  void appendfile(const char *name,const char *filename);
  void append();
  void remove(const char *name);
private:
  bool mystery=false;
};
#endif
