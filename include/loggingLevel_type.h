// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <open62541/plugin/log.h>
#include <open62541/server.h>
/*
 * loggingLevel_type.h
 *
 *  Created on: Jun 12, 2024
 *      Author: Klaus Zenker (HZDR)
 */

// We can not use UA_LogLevel because the enum type expects 0,1,... and UA_LogLevel uses 100,200,...
typedef enum {
  UA_LOGGINGLEVEL_TRACE = 0,
  UA_LOGGINGLEVEL_DEBUG = 1,
  UA_LOGGINGLEVEL_INFO = 2,
  UA_LOGGINGLEVEL_WARNING = 3,
  UA_LOGGINGLEVEL_ERROR = 4,
  UA_LOGGINGLEVEL_FATAL = 5,
  __UA_LOGGINGLEVEL_FORCE32BIT = 0x7fffffff
} UA_LoggingLevel;

UA_STATIC_ASSERT(sizeof(UA_LoggingLevel) == sizeof(UA_Int32), enum_must_be_32bit);

static UA_INLINE UA_LogLevel toLogLevel(const UA_LoggingLevel l) {
  UA_LogLevel dest;
  switch(l) {
    case UA_LOGGINGLEVEL_TRACE:
      dest = UA_LOGLEVEL_TRACE;
      break;
    case UA_LOGGINGLEVEL_DEBUG:
      dest = UA_LOGLEVEL_DEBUG;
      break;
    case UA_LOGGINGLEVEL_INFO:
      dest = UA_LOGLEVEL_INFO;
      break;
    case UA_LOGGINGLEVEL_WARNING:
      dest = UA_LOGLEVEL_WARNING;
      break;
    case UA_LOGGINGLEVEL_ERROR:
      dest = UA_LOGLEVEL_ERROR;
      break;
    case UA_LOGGINGLEVEL_FATAL:
      dest = UA_LOGLEVEL_FATAL;
      break;
    default:
      dest = UA_LOGLEVEL_INFO;
      break;
  }
  return dest;
}

static UA_INLINE UA_LoggingLevel toLoggingLevel(const UA_LogLevel l) {
  UA_LoggingLevel dest;
  switch(l) {
    case UA_LOGLEVEL_TRACE:
      dest = UA_LOGGINGLEVEL_TRACE;
      break;
    case UA_LOGLEVEL_DEBUG:
      dest = UA_LOGGINGLEVEL_DEBUG;
      break;
    case UA_LOGLEVEL_INFO:
      dest = UA_LOGGINGLEVEL_INFO;
      break;
    case UA_LOGLEVEL_WARNING:
      dest = UA_LOGGINGLEVEL_WARNING;
      break;
    case UA_LOGLEVEL_ERROR:
      dest = UA_LOGGINGLEVEL_ERROR;
      break;
    case UA_LOGLEVEL_FATAL:
      dest = UA_LOGGINGLEVEL_FATAL;
      break;
    default:
      dest = UA_LOGGINGLEVEL_INFO;
      break;
  }
  return dest;
}

static const UA_DataType LoggingLevelType = {
    UA_TYPENAME("LoggingLevels")                                            /* .typeName */
    {1, UA_NODEIDTYPE_STRING, {.string = {13, (UA_Byte*)"LoggingLevels"}}}, /* .typeId */
    {0, UA_NODEIDTYPE_NUMERIC, {0}},                                        /* .binaryEncodingId */
    sizeof(UA_LoggingLevel),                                                /* .memSize */
    UA_DATATYPEKIND_ENUM,                                                   /* .typeKind */
    true,                                                                   /* .pointerFree */
    UA_BINARY_OVERLAYABLE_INTEGER,                                          /* .overlayable */
    0,                                                                      /* .membersSize */
    NULL                                                                    /* .members */
};
static UA_INLINE void UA_LoggingLevel_init(UA_LoggingLevel* p) {
  memset(p, 0, sizeof(UA_LoggingLevel));
}

static UA_INLINE UA_LoggingLevel* UA_LoggingLevel_new(void) {
  return (UA_LoggingLevel*)UA_new(&LoggingLevelType);
}

static UA_INLINE UA_StatusCode UA_LoggingLevel_copy(const UA_LoggingLevel* src, UA_LoggingLevel* dst) {
  return UA_copy(src, dst, &LoggingLevelType);
}

UA_DEPRECATED static UA_INLINE void UA_LoggingLevel_deleteMembers(UA_LoggingLevel* p) {
  UA_clear(p, &LoggingLevelType);
}

static UA_INLINE void UA_LoggingLevel_clear(UA_LoggingLevel* p) {
  UA_clear(p, &LoggingLevelType);
}

static UA_INLINE void UA_LoggingLevel_delete(UA_LoggingLevel* p) {
  UA_delete(p, &LoggingLevelType);
}
static UA_INLINE UA_Boolean UA_LoggingLevel_equal(const UA_LoggingLevel* p1, const UA_LoggingLevel* p2) {
  return (UA_order(p1, p2, &LoggingLevelType) == UA_ORDER_EQ);
}
