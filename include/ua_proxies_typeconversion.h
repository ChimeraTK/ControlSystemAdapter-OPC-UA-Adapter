/*
 * Copyright (c) 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
 * Chair for Process Systems Engineering
 * Technical University of Dresden
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

#ifndef HAVE_UA_PROXIES_TYPECONVERSION_H
#define HAVE_UA_PROXIES_TYPECONVERSION_H

/* Helpers for type converstion and such */
#define UA_STRING_TO_CPPSTRING_COPY(_p_uastring, _p_cppstring) do { \
UA_String *ua_url = (UA_String *) _p_uastring; \
char *ua_str_tmp = (char *) malloc(ua_url->length+1); \
memset(ua_str_tmp, 0, ua_url->length+1); \
strncpy(ua_str_tmp, (char *) ua_url->data, ua_url->length); \
*(_p_cppstring) = ua_str_tmp; \
free(ua_str_tmp); \
} while(0); \

/* UASTRING_TO_CPPSTRING(UA_String s_ua, char *s_c)
 * 
 * Create a new buffer s_c that will contain s_ua
 */
#define UASTRING_AS_CSTRING(_p_uastring, _p_strbuffer) { \
_p_strbuffer = (char *) malloc(_p_uastring.length + 1); \
memset(_p_strbuffer, 0, _p_uastring.length + 1); \
memcpy(_p_strbuffer, _p_uastring.data,_p_uastring.length); \
}

/* UASTRING_TO_CPPSTRING(UA_String s_ua, std::string s_cpp)
 * 
 * Copy contents of s_ua into s_cpp
 */
#define UASTRING_TO_CPPSTRING(_p_uastring, _p_cppstring) do { \
char *s;\
UASTRING_AS_CSTRING(_p_uastring, s) \
_p_cppstring = s;\
free(s);\
} while(0);

/* CPPSTRING_TO_UASTRING(UA_String s_ua, std::string s_cpp)
 * 
 * Copy contents of s_cpp into s_ua
 */
#define CPPSTRING_TO_UASTRING(_p_uastring, _p_cppstring) {\
const char *s  = _p_cppstring.c_str(); \
_p_uastring.length = _p_cppstring.length(); \
_p_uastring.data = (UA_Byte *) malloc(_p_uastring.length); \
memcpy(_p_uastring.data, s, _p_uastring.length); \
}

#define NODE_BROWSENAME_AS_STRING(_p_server, _p_nodeid, _p_strbuffer) { \
UA_QualifiedName _p_tmpName; \
UA_Server_readBrowseName(_p_server, _p_nodeid, &_p_tmpName); \
UASTRING_AS_STRING(_p_tmpName.name,  _p_strbuffer); \
}

#define NODE_DISPLAYNAME_AS_STRING(_p_server, _p_nodeid, _p_strbuffer) { \
UA_LocalizedText _p_tmpName; \
UA_Server_readDisplayName(_p_server, _p_nodeid, &_p_tmpName); \
UASTRING_AS_STRING(_p_tmpName.text,  _p_strbuffer); \
}

#define NODE_DESCRIPTION_AS_STRING(_p_server, _p_nodeid, _p_strbuffer) { \
UA_LocalizedText _p_tmpName; \
UA_Server_readDisplayName(_p_server, _p_nodeid, &_p_tmpName); \
UASTRING_AS_STRING(_p_tmpName.text,  _p_strbuffer); \
}



#endif //#ifndef HAVE_UA_PROXIES_TYPECONVERSION_H
