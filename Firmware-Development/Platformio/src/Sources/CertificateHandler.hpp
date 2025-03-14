#pragma once
#include "SPIFFS.h"

#define FILE_TITLE "/server.crt"
#define FILE_JSON "/config.json"

class CertificateHandler 
{
  public:
    void setup(); 
    void isExist(); 
    void write(const char* content); 
    void print(); 
    char* get(const char* filename);
};

extern CertificateHandler certificateHandler;