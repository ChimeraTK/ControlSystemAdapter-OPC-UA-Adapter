// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2023 Andreas Ebner <andreas.ebner@iosb.fraunhofer.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <open62541/server.h>

extern UA_StatusCode csa_namespace_init(UA_Server* server);
extern void csa_namespace_add_LoggingLevelEnumType(UA_Server* server, char* enumName);
extern UA_StatusCode csa_namespace_add_LoggingLevelValues(UA_Server* server);
extern UA_StatusCode csa_namespace_add_additional_variable(UA_Server* server);
