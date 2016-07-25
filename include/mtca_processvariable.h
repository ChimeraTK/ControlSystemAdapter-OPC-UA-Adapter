/*
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef MTCA_PROCESSVARIABLE_H
#define MTCA_PROCESSVARIABLE_H

#include "ua_mapped_class.h"

typedef enum { PROCESSVARIABLE_TYPE_VOID,
  PROCESSVARIABLE_TYPE_INT32,
  PROCESSVARIABLE_TYPE_UINT32,
  PROCESSVARIABLE_TYPE_STRING
} variableType;

class mtca_processvariable :  ua_mapped_class
{
private:
  std::string name;
  std::string value;
  std::string type;
  uint32_t timeStamp;
  
  //variableType type; // unused
  
  UA_StatusCode mapSelfToNamespace();
  void mtca_processvariable_globalinit(std::string name, std::string type, std::string value, uint32_t timeStamp);
public:
  mtca_processvariable(UA_Server *server, UA_NodeId basenodeid, std::string name, std::string value, std::string type, uint32_t timeStamp);
  ~mtca_processvariable();
  
  void setName(std::string name);
  std::string getName();
  
  void setType(std::string type);
  std::string getType();
  
  void setValue(std::string value);
  std::string getValue();
  
  void setTimeStamp(uint32_t timeStamp);
  uint32_t getTimeStamp();
};

#endif // MTCA_PROCESSVARIABLE_H
