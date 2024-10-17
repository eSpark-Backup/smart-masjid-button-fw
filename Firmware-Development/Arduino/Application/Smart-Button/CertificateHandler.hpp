#pragma once
#include "SPIFFS.h"

#define FILE_TITLE "/server.crt"

class CertificateHandler 
{
  public:
    void setup(); 
    void isExist(); 
    void write(const char* content); 
    void print(); 
    char* get();
};
