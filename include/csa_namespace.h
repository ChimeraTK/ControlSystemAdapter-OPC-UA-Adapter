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
 * Copyright (c) 2023 Andreas Ebner <andreas.ebner@iosb.fraunhofer.de>
 */
#pragma once

#include <open62541/server.h>

extern UA_StatusCode csa_namespace_init(UA_Server* server);
extern void csa_namespace_add_LoggingLevelEnumType(UA_Server* server, char* enumName);
extern UA_StatusCode csa_namespace_add_LoggingLevelValues(UA_Server* server, const UA_NodeId* parent);
extern UA_StatusCode csa_namespace_add_additional_variable(UA_Server* server);
