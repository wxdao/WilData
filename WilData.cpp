#include "WilData.hpp"
#include "waaaaxd.hpp"
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <zlib.h>

const char *WilData::getfilename(){
  return fn;
}
const char *WilData::getmessage(){
  return meg.c_str();
}
vector<pair<string,WilData::datainfo>> WilData::getvdi(){
                                       return vdi;
}
WilData::datainfo WilData::getdi(const char *name){
  for(pair<string,WilData::datainfo> p : vdi)
    {
      if(p.first==name)
        {
          return p.second;
        }
    }
  throw 0;
}

//*****************R***********************
size_t WilData_R::getdata(const char *name, byte *data){
  datainfo di=getdi(name);
  FILE *f=fopen(fn,"rb");
  fseek(f,di.addr,SEEK_SET);
  byte *de=(byte*)malloc(di.dsize);
  long unsigned int len=di.size;
  fread(de,1,di.dsize,f);
  byte hasht[20];
  waxd::hash::sha1byte_b(de,di.dsize,hasht);
  if(memcmp(hasht,di.hash,20)!=0){
      memset(data,0,di.size);
      free(de);
      data=nullptr;
      return 0;
    }
  uncompress(data,&len,de,di.dsize);
  free(de);
  return 1;
}

void WilData_R::appenddata(const char *name, byte *data, long size){
  if(mystery==true){
      datainfo di;
      memset(&di,0,sizeof(di));
      di.addr=0;
      di.size=size;
      uLongf fuck=compressBound(size);
      byte* dest=(byte*)malloc(fuck);
      compress2(dest,&fuck,data,size,9);
      di.dsize=fuck;
      waxd::hash::sha1byte_b(dest,di.dsize,di.hash);
      vdi.push_back(make_pair(name,di));
      vd.push_back(make_pair(dest,di.dsize));
    }else{
      for(pair<string,WilData::datainfo> p : vdi){
          byte *b=(byte*)malloc(p.second.size);
          getdata(p.first.c_str(),b);
          uLongf fuck=compressBound(p.second.size);
          byte* dest=(byte*)malloc(fuck);
          compress2(dest,&fuck,b,p.second.size,9);
          free(b);
          vd.push_back(make_pair(dest,fuck));
        }
      datainfo di;
      memset(&di,0,sizeof(di));
      di.addr=0;
      di.size=size;
      uLongf fuck=compressBound(size);
      byte* dest=(byte*)malloc(fuck);
      compress2(dest,&fuck,data,size,9);
      di.dsize=fuck;
      waxd::hash::sha1byte_b(dest,di.dsize,di.hash);
      vdi.push_back(make_pair(name,di));
      vd.push_back(make_pair(dest,di.dsize));
    }
}

void WilData_R::appendfile(const char *name, const char *filename){
  size_t fs=waxd::getfilesize(filename);
  byte *filebuff=(byte*)malloc(fs);
  FILE *f=fopen(filename,"rb");
  fread(filebuff,1,fs,f);
  fclose(f);
  appenddata(name,filebuff,fs);
  free(filebuff);
}

void WilData_R::append(){
  FILE *f=fopen(fn,"wb");
  fwrite("#",1,1,f);

  size_t len=meg.length();
  fwrite(&len,1,sizeof(len),f);
  fwrite(meg.c_str(),1,len,f);
  byte meghash[20];
  waxd::hash::sha1byte_b((byte*)meg.c_str(),len,meghash);
  fwrite(meghash,1,20,f);
  fwrite(&ver,1,sizeof(ver),f);
  fwrite(&sign,1,sizeof(sign),f);
  vector<long> addrlist;
  for(pair<string,datainfo> p : vdi){
      len=p.first.length();
      fwrite(&len,1,sizeof(len),f);
      fwrite(p.first.c_str(),1,len,f);
      addrlist.push_back(ftell(f));
      fwrite(&p.second,1,sizeof(datainfo),f);
    }

  long datastart=ftell(f);
  long j=0;
  for(pair<byte*,long> p : vd){
      long cur =ftell(f);
      fseek(f,addrlist[j],SEEK_SET);
      fwrite(&cur,1,sizeof(long),f);
      fseek(f,cur,SEEK_SET);
      fwrite(p.first,1,p.second,f);
      ++j;
      free(p.first);
    }
  fwrite(&datastart,1,sizeof(long),f);
  fclose(f);
  mystery=false;
  vd.clear();
}

void WilData_R::remove(const char *name){
  long i=0;
  vd.clear();
  mystery=true;
  for(pair<string,WilData::datainfo> p : vdi){
      if(p.first==string(name)){
          vdi.erase(vdi.begin()+i);
          ++i;
          break;
        }
    }


  for(pair<string,WilData::datainfo> p : vdi){
      byte *b=(byte*)malloc(p.second.size);
      getdata(p.first.c_str(),b);
      uLongf fuck=compressBound(p.second.size);
      byte* dest=(byte*)malloc(fuck);
      compress2(dest,&fuck,b,p.second.size,9);
      free(b);
      vd.push_back(make_pair(dest,fuck));
      ++i;
    }
  append();
}

WilData_R::WilData_R(const char *filename){
  open(filename);
}
bool WilData_R::open(const char *filename){
  fn=filename;
  FILE *f=fopen(fn,"rb");
  long startaddr;
  char tmp;
  do{
      fread(&tmp,1,1,f);
    }while(tmp!='#');
  startaddr=ftell(f)-1;
  size_t len;
  fread(&len,1,sizeof(len),f);
  char megtmp[len+1];
  megtmp[len]=0;
  fread(megtmp,1,len,f);
  byte meghash[20],meghasht[20];
  waxd::hash::sha1byte_b((byte*)megtmp,len,meghasht);
  fread(meghash,1,20,f);
  if(memcmp(meghasht,meghash,20))
    return false;
  meg=string(megtmp);
  fread(&ver,1,sizeof(ver),f);
  fread(&sign,1,sizeof(sign),f);

  long cur =ftell(f);
  fseek(f,-sizeof(long),SEEK_END);
  long diend;
  fread(&diend,1,sizeof(diend),f);
  fseek(f,cur,SEEK_SET);
  while(ftell(f)!=diend){
      fread(&len,1,sizeof(len),f);
      char nametmp[len+1];
      nametmp[len]=0;
      fread(nametmp,1,len,f);
      datainfo di;
      fread(&di,1,sizeof(di),f);
      di.dsize+=startaddr;
      di.size+=startaddr;
      vdi.push_back(make_pair(string(nametmp),di));
    }
  fclose(f);
  return true;
}

//*****************W***********************
WilData_W::WilData_W(const char *filename){
  open(filename);
}
bool WilData_W::open(const char *filename){
  fn=filename;
  return true;
}
void WilData_W::setmessage(const char *m){
  WilData::meg=m;
}
void WilData_W::adddata(const char *name, byte *data, long size){
  datainfo di;
  memset(&di,0,sizeof(di));
  di.addr=0;
  di.size=size;
  byte* dest=(byte*)malloc(di.size);
  uLongf fuck=compressBound(size);
  compress2(dest,&fuck,data,size,9);
  di.dsize=fuck;
  waxd::hash::sha1byte_b(dest,di.dsize,di.hash);
  vdi.push_back(make_pair(name,di));
  vd.push_back(make_pair(dest,di.dsize));
}
void WilData_W::addfile(const char *name, const char *filename){
  size_t fs=waxd::getfilesize(filename);
  byte *filebuff=(byte*)malloc(fs);
  FILE *f=fopen(filename,"rb");
  fread(filebuff,1,fs,f);
  fclose(f);
  adddata(name,filebuff,fs);
  free(filebuff);
}

void WilData_W::write(){
  FILE *f=fopen(fn,"wb");
  fwrite("#",1,1,f);

  size_t len=meg.length();
  fwrite(&len,1,sizeof(len),f);
  fwrite(meg.c_str(),1,len,f);
  byte meghash[20];
  waxd::hash::sha1byte_b((byte*)meg.c_str(),len,meghash);
  fwrite(meghash,1,20,f);
  fwrite(&ver,1,sizeof(ver),f);
  fwrite(&sign,1,sizeof(sign),f);
  vector<long> addrlist;
  for(pair<string,datainfo> p : vdi){
      len=p.first.length();
      fwrite(&len,1,sizeof(len),f);
      fwrite(p.first.c_str(),1,len,f);
      addrlist.push_back(ftell(f));
      fwrite(&p.second,1,sizeof(datainfo),f);
    }

  long datastart=ftell(f);
  long j=0;
  for(pair<byte*,long> p : vd){
      long cur =ftell(f);
      fseek(f,addrlist[j],SEEK_SET);
      fwrite(&cur,1,sizeof(long),f);
      fseek(f,cur,SEEK_SET);
      fwrite(p.first,1,p.second,f);
      ++j;
      free(p.first);
    }
  fwrite(&datastart,1,sizeof(long),f);
  fclose(f);

}
