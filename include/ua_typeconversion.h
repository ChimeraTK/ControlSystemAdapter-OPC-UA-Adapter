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

#ifndef HAVE_UA_TYPECONVERSION_H
#define HAVE_UA_TYPECONVERSION_H

#include <open62541/server.h>

#include <string>

/* Helpers for type converstion and such */
#define UA_STRING_TO_CPPSTRING_COPY(_p_uastring, _p_cppstring)                                                         \
  do {                                                                                                                 \
    UA_String* ua_url = (UA_String*)_p_uastring;                                                                       \
    char* ua_str_tmp = (char*)malloc(ua_url->length + 1);                                                              \
    memset(ua_str_tmp, 0, ua_url->length + 1);                                                                         \
    strncpy(ua_str_tmp, (char*)ua_url->data, ua_url->length);                                                          \
    *(_p_cppstring) = ua_str_tmp;                                                                                      \
    free(ua_str_tmp);                                                                                                  \
  } while(0);

/* UASTRING_TO_CPPSTRING(UA_String s_ua, char *s_c)
 *
 * Create a new buffer s_c that will contain s_ua
 */
#define UASTRING_AS_CSTRING(_p_uastring, _p_strbuffer)                                                                 \
  {                                                                                                                    \
    _p_strbuffer = (char*)malloc(_p_uastring.length + 1);                                                              \
    memset(_p_strbuffer, 0, _p_uastring.length + 1);                                                                   \
    memcpy(_p_strbuffer, _p_uastring.data, _p_uastring.length);                                                        \
  }

/* UASTRING_TO_CPPSTRING(UA_String s_ua, std::string s_cpp)
 *
 * Copy contents of s_ua into s_cpp
 */
#define UASTRING_TO_CPPSTRING(_p_uastring, _p_cppstring)                                                               \
  do {                                                                                                                 \
    char* s;                                                                                                           \
    UASTRING_AS_CSTRING(_p_uastring, s)                                                                                \
    _p_cppstring.assign(s, _p_uastring.length);                                                                        \
    free(s);                                                                                                           \
  } while(0);

/* CPPSTRING_TO_UASTRING(UA_String s_ua, std::string s_cpp)
 *
 * Copy contents of s_cpp into s_ua
 */
static inline UA_String CPPSTRING_TO_UASTRING(std::string& s) {
  char* buf = (char*)malloc(s.length());
  if(!buf) return UA_STRING_NULL;
  strncpy(buf, (char*)s.c_str(), s.length());
  return {s.length(), (UA_Byte*)buf};
}

#define CPPSTRING_TO_UASTRING_NEW(_p_uastring, _p_cppstring)                                                           \
  {                                                                                                                    \
    char* s = (char*)malloc(_p_cppstring->length() + 1);                                                               \
    strncpy(s, (char*)_p_cppstring->c_str(), _p_cppstring->length());                                                  \
    _p_uastring.length = _p_cppstring->length();                                                                       \
    _p_uastring.data = (UA_Byte*)malloc(_p_uastring.length);                                                           \
    memcpy(_p_uastring.data, s, _p_uastring.length);                                                                   \
    free(s);                                                                                                           \
  }

#define NODE_BROWSENAME_AS_STRING(_p_server, _p_nodeid, _p_strbuffer)                                                  \
  {                                                                                                                    \
    UA_QualifiedName _p_tmpName;                                                                                       \
    UA_Server_readBrowseName(_p_server, _p_nodeid, &_p_tmpName);                                                       \
    UASTRING_AS_STRING(_p_tmpName.name, _p_strbuffer);                                                                 \
  }

#define NODE_DISPLAYNAME_AS_STRING(_p_server, _p_nodeid, _p_strbuffer)                                                 \
  {                                                                                                                    \
    UA_LocalizedText _p_tmpName;                                                                                       \
    UA_Server_readDisplayName(_p_server, _p_nodeid, &_p_tmpName);                                                      \
    UASTRING_AS_STRING(_p_tmpName.text, _p_strbuffer);                                                                 \
  }

#define NODE_DESCRIPTION_AS_STRING(_p_server, _p_nodeid, _p_strbuffer)                                                 \
  {                                                                                                                    \
    UA_LocalizedText _p_tmpName;                                                                                       \
    UA_Server_readDisplayName(_p_server, _p_nodeid, &_p_tmpName);                                                      \
    UASTRING_AS_STRING(_p_tmpName.text, _p_strbuffer);                                                                 \
  }

#endif //#ifndef HAVE_UA_TYPECONVERSION_H
