
#include"CertificateHandler.hpp"

// Method to initialize SPIFFS
void CertificateHandler::setup() 
{
  if (!SPIFFS.begin(true)) 
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  } 
  else 
  {
    Serial.println("SPIFFS mounted successfully");
  }
}

// Method to check if the file exists
void CertificateHandler::isExist() 
{
  if (SPIFFS.exists(FILE_TITLE)) 
  {
    Serial.println("File exists");
  } 
  else 
  {
    Serial.println("File does not exist");
  }
}

// Method to write new content to the file
void CertificateHandler::write(const char* content) 
{
  File file = SPIFFS.open(FILE_TITLE, FILE_WRITE);
  if (!file) 
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(content);
  file.close();
  Serial.println("File written successfully");
}

// Method to read and print the content of the file
void CertificateHandler::print() 
{
  File file = SPIFFS.open(FILE_TITLE, FILE_READ);
  if (!file) 
  {
    Serial.println("Failed to open file for reading");
    return;
  }
   
  size_t bufferSize = file.size();  // Get the size of the file
  if (bufferSize == 0) 
  {
    Serial.println("File is empty");
    file.close();
    return;
  }
  char buffer[bufferSize + 1];   // +1 for null terminator

  Serial.println("File Content:");

  while (file.available()) 
  {
    // Read up to bufferSize bytes from the file
    size_t bytesRead = file.readBytes(buffer, bufferSize);
    buffer[bytesRead] = '\0';
    Serial.print(buffer);  
  }
  file.close();
  Serial.println();
}

char* CertificateHandler::get() {
  File file = SPIFFS.open(FILE_TITLE, FILE_READ);
  if (!file) 
  {
    Serial.println("Failed to open file for reading");
    return nullptr;
  }

  size_t size = file.size();  // Get the size of the file
  if (size == 0) 
  {
    Serial.println("File is empty");
    file.close();
    return nullptr;
  }
  char* content = new char[size + 1]; 
  if (content == nullptr) 
  {
    Serial.println("Failed to allocate memory");
    file.close();
    return nullptr;
  }

  file.readBytes(content, size);
  content[size] = '\0';

  file.close();
  return content;
}
