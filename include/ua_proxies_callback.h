/* 
 * This file is part of ChimeraTKs ControlSystem-OPC-UA-Adapter.
 *
 * ChimeraTKs ControlSystem-OPC-UA-Adapter is free software: you can 
 * redistribute it and/or modify it under the terms of the Lesser GNU 
 * General Public License as published by the Free Software Foundation, 
 * either version 3 of the License, or (at your option) any later version.
 *
 * ChimeraTKs ControlSystem-OPC-UA-Adapter is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty ofMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 * See the Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see https://www.gnu.org/licenses/lgpl.html
 * 
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Copyright (c) 2016 Julian Rahm  <Julian.Rahm@tu-dresden.de>
 */

#ifndef HAVE_UA_PROXIES_CALLBACK_H
#define HAVE_UA_PROXIES_CALLBACK_H

#include "sys/time.h"
#include "stdio.h"
#include <string>

#define C_MACRO_CONCAT_NOEXP(A,B) A ## B
#define C_MACRO_CONCAT(A,B) C_MACRO_CONCAT_NOEXP(A, B) // This causes macros to be expanded

/* Function call proxy generator
 * UA_CALLPROXY: Generate callback for the ua_stack. Calls CLASS_F in CLASS_P (this->) is implied.
 * UA_CALLPROXY_NAME Return the name of the call proxy
 */
// Generate Call-Through functions as stack callbacks
#define UA_CALLPROXY_NAME(_CLASS_P, _CLASS_F) ua_callproxy_##_CLASS_P##_##_CLASS_F
//#define UA_CALLPROXY_TABLENAME(_CLASS_P, _CLASS_F) C_MACRO_CONCAT( UA_CALLPROXY_NAME(_CLASS_P,_CLASS_F) , _InstanceLookUpTable)
#define UA_CALLPROXY_TABLE(_CLASS_P, _CLASS_F) & UA_CALLPROXY_TABLENAME(_CLASS_P, _CLASS_F)

#define UA_CALLPROXY(_CLASS_P, _CLASS_F) \
UA_FunctionCall_InstanceLookUpTable UA_CALLPROXY_TABLENAME(_CLASS_P, _CLASS_F); \
UA_StatusCode UA_CALLPROXY_NAME(_CLASS_P,_CLASS_F)(void *methodHandle, const UA_NodeId objectId, size_t inputSize, const UA_Variant *input, size_t outputSize, UA_Variant *output){ \
  for (UA_FunctionCall_InstanceLookUpTable::iterator j=UA_CALLPROXY_TABLENAME(_CLASS_P, _CLASS_F).begin(); j != UA_CALLPROXY_TABLENAME(_CLASS_P, _CLASS_F).end(); j++) \
    if(UA_NodeId_equal(&(*(j))->classObjectId, &objectId)) { \
      _CLASS_P *theClass = static_cast<_CLASS_P *>( (*(j))->classInstance ); \
      return theClass->_CLASS_F(inputSize, input, outputSize, output); \
    } \
  return UA_STATUSCODE_GOOD; \
}

/* Generators for Valuesource read callbacks
 * Anyone who finds a way to do this using templates gets a cookie!
 * These callbacks are passed as functions to open62541 dataValue variables for read operations. 
 * They invoke the appropriate getXY() function of the object passed as handle.
 */

/* Prototype wrappers */
// Generate Proxy names
#define UA_RDPROXY_NAME(_p_class, _p_method) ua_readproxy_ ##_p_class## _ ##_p_method
#define UA_WRPROXY_NAME(_p_class, _p_method) ua_writeproxy_ ##_p_class## _ ##_p_method

// Generate Proxy tails (common function end)
#define UA_RDPROXY_TAIL() \
value->hasValue = UA_TRUE; \
if (includeSourceTimeStamp) { \
value->sourceTimestamp =  thisObj->getSourceTimeStamp(); \
value->hasSourceTimestamp = UA_TRUE; \
}\
return UA_STATUSCODE_GOOD; } // Ends Fnct

#define UA_WRPROXY_TAIL(_p_method) \
return UA_STATUSCODE_GOOD; } // End Fnct

// Generate Proxy heads (common function starts)
#define UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_StatusCode UA_RDPROXY_NAME(_p_class, _p_method) (void *handle, const UA_NodeId nodeid, UA_Boolean includeSourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value) { \
_p_class *thisObj = static_cast<_p_class *> (handle); \

#define UA_WRPROXY_HEAD(_p_class, _p_method)  \
UA_StatusCode UA_WRPROXY_NAME(_p_class, _p_method) (void *handle, const UA_NodeId nodeid,const UA_Variant *data, const UA_NumericRange *range) {\
_p_class *theClass = static_cast<_p_class *> (handle);

// Generator for Simple Read bodies (type cast)
#define UA_RDPROXY_SIMPLEBODY(_p_method, _p_ctype, _p_uatype) \
_p_ctype ua_val = thisObj->_p_method(); \
UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[_p_uatype]); \

#define UA_WRPROXY_SIMPLEBODY(_p_method, _p_ctype) \
_p_ctype value = (_p_ctype) *((_p_ctype *) data->data); \
theClass->_p_method (value);

#define UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, _p_ctype) \
_p_ctype* v = (_p_ctype *) data->data; \
std::vector<_p_ctype> vectorizedValue(data->arrayLength); \
for(uint32_t i=0; i < vectorizedValue.size(); i++) vectorizedValue.at(i) = v[i]; \
theClass->_p_method(vectorizedValue); \

#define UA_WRPROXY_SIMPLEBODY_ARRAY_STRING(_p_method, _p_ctype) \
//UA_String* v = (UA_String*) data->data; \
//std::vector<std::string> vectorizedValue(data->arrayLength); \
//for(uint32_t i=0; i < vectorizedValue.size(); i++) { \
//	std::string c_string; \
//	UASTRING_TO_CPPSTRING(*v[i] , c_string); \
//	vectorizedValue.at(i) = (std::string)c_string; \
//} \
//theClass->_p_method(vectorizedValue); \

#define UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, _p_ctype, _p_uatype) \
UA_Variant_setArrayCopy(&value->value, (_p_ctype *) thisObj->_p_method().data(), thisObj->_p_method().size(), &UA_TYPES[_p_uatype]); \
UA_NumericRange arrayRange; \
arrayRange.dimensionsSize = 1; \
UA_NumericRangeDimension scalarThisDimension = (UA_NumericRangeDimension){.min = 0, .max = (unsigned)thisObj->_p_method().size()}; \
arrayRange.dimensions = &scalarThisDimension; \
UA_Variant_setRangeCopy(&value->value, (_p_ctype *) thisObj->_p_method().data(), thisObj->_p_method().size(), arrayRange); \

#define UA_RDPROXY_SIMPLEBODY_ARRAY_STRING(_p_method, _p_ctype, _p_uatype) \
//std::basic_string<char>* vectorizedValue; \
//vectorizedValue = thisObj->_p_method().data(); \
//UA_String ua_string_vector[thisObj->_p_method().size()]; \
//for(uint32_t i=0; i < thisObj->_p_method().size(); i++) { \
//	UA_String ua_val; \
//	CPPSTRING_TO_UASTRING_NEW(ua_val, vectorizedValue); \
//	ua_string_vector[i] = ua_val; \
//} \
//UA_Variant_setArrayCopy(&value->value, ua_string_vector, thisObj->_p_method().size(), &UA_TYPES[UA_TYPES_STRING]); \
//UA_NumericRange arrayRange; \
//arrayRange.dimensionsSize = 1; \
//UA_NumericRangeDimension scalarThisDimension = (UA_NumericRangeDimension){.min = 0, .max = (unsigned)thisObj->_p_method().size()}; \
//arrayRange.dimensions = &scalarThisDimension; \
//UA_Variant_setRangeCopy(&value->value, (_p_ctype *) thisObj->_p_method().data(), thisObj->_p_method().size(), arrayRange); \

// Typed Function Protoypes with datatype specific stuff
// Readproxies:
#define UA_RDPROXY_STRING(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
string temp = thisObj->_p_method();        \
UA_String ua_val = CPPSTRING_TO_UASTRING(temp); \
UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_STRING]); \
UA_String_deleteMembers(&ua_val); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_LOCALIZEDTEXT(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_LocalizedText ua_val; \
std::tuple<std::string, std::string> val = thisObj->_p_method(); \
std::string locale = std::get<0>(val); \
std::string text   = std::get<1>(val); \
ua_val.locale = CPPSTRING_TO_UASTRING(locale); \
ua_val.text = CPPSTRING_TO_UASTRING(text); \
UA_Variant_setScalarCopy(&value->value, &ua_val, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); \
UA_LocalizedText_deleteMembers(&ua_val); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_INT32(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, int32_t, UA_TYPES_INT32); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_UINT32(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, uint32_t, UA_TYPES_UINT32); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_INT16(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, int16_t, UA_TYPES_INT16); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_UINT16(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, uint16_t, UA_TYPES_UINT16); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_INT8(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, int8_t, UA_TYPES_SBYTE); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_UINT8(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, uint8_t, UA_TYPES_BYTE); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_BOOL(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, UA_Boolean, UA_TYPES_BOOLEAN); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_FLOAT(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, float, UA_TYPES_FLOAT); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_DOUBLE(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, double, UA_TYPES_DOUBLE); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_DATETIME(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, long long int, UA_TYPES_DATETIME); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_NODEID(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY(_p_method, UA_NodeId, UA_TYPES_NODEID); \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_INT8(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, int8_t, UA_TYPES_SBYTE) \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_UINT8(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, uint8_t, UA_TYPES_BYTE) \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_INT16(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, int16_t, UA_TYPES_INT16) \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_UINT16(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, uint16_t, UA_TYPES_UINT16) \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_INT32(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, int32_t, UA_TYPES_INT32) \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_UINT32(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, uint32_t, UA_TYPES_UINT32) \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_FLOAT(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, float, UA_TYPES_FLOAT) \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_DOUBLE(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY(_p_method, double, UA_TYPES_DOUBLE) \
UA_RDPROXY_TAIL()

#define UA_RDPROXY_ARRAY_STRING(_p_class, _p_method) \
UA_RDPROXY_HEAD(_p_class, _p_method) \
UA_RDPROXY_SIMPLEBODY_ARRAY_STRING(_p_method, string, UA_TYPES_STRING) \
UA_RDPROXY_TAIL()

// Writeproxies:
#define UA_WRPROXY_STRING(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
std::string cpps; \
UASTRING_TO_CPPSTRING( ((UA_String) *((UA_String *) data->data)) , cpps); \
theClass->_p_method(cpps); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_LOCALIZEDTEXT(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
std::string locale; \
UASTRING_TO_CPPSTRING( ((UA_String) *((UA_String *) &((UA_LocalizedText *) data->data)->locale)) , locale); \
std::string text; \
UASTRING_TO_CPPSTRING( ((UA_String) *((UA_String *) &((UA_LocalizedText *) data->data)->text)) , text); \
theClass->_p_method(std::make_tuple(locale, text)); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_INT8(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, uint32_t); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_UINT8(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, uint32_t); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_INT16(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, uint32_t); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_UINT16(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, uint32_t); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_INT32(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, uint32_t); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_UINT32(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, uint32_t); \
UA_WRPROXY_TAIL()
    
#define UA_WRPROXY_BOOL(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, bool); \
UA_WRPROXY_TAIL()
    
#define UA_WRPROXY_FLOAT(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, float); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_DOUBLE(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, double); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_DATETIME(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, UA_DateTime); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_NODEID(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY(_p_method, UA_NodeId); \
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_INT8(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, int8_t)\
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_UINT8(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, uint8_t)\
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_INT16(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, int16_t)\
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_UINT16(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, uint16_t)\
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_INT32(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, int32_t)\
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_UINT32(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, uint32_t)\
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_FLOAT(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, float)\
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_DOUBLE(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY(_p_method, double)\
UA_WRPROXY_TAIL()

#define UA_WRPROXY_ARRAY_STRING(_p_class, _p_method) \
UA_WRPROXY_HEAD(_p_class, _p_method) \
UA_WRPROXY_SIMPLEBODY_ARRAY_STRING(_p_method, _p_ctype) \
UA_WRPROXY_TAIL()

#endif //HAVE_UA_PROXIES_CALLBACK_H
