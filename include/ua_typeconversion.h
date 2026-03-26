// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <open62541/server.h>

#include <string>

/* Helpers for type conversion and such */
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
