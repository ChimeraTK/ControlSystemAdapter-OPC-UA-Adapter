/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MTCA_PROCESSSCALAR_H
#define MTCA_PROCESSSCALAR_H

#include "ChimeraTK/ControlSystemAdapter/TimeStamp.h"
#include "ua_mapped_class.h"
#include "mtca_processvariable.h"
#include "mtca_timestamp.h"

#include <iostream>
#include <string>

/*
 * TODO timeStamp should be from type mtca_timestamp
 */

class mtca_processscalar : ua_mapped_class {
private:
	std::string		name;
	std::string		valueType;
	std::string		value;
	ChimeraTK::TimeStamp	timeStamp;
	
public:
	~mtca_processscalar();
	mtca_processscalar(UA_Server *server, UA_NodeId baseNodeId, std::string name, std::string value, std::type_info const &valueType, ChimeraTK::TimeStamp timeStamp);
	
	void setName(std::string name);
	std::string getName();
	
	void setType(std::string valueType);
	std::string getType();
	
	void setValue(std::string value);
	std::string getValue();
  
	void setTimeStamp(ChimeraTK::TimeStamp timeStamp);
	ChimeraTK::TimeStamp getTimeStamp();

	
	void mtca_processvariable_globalinit(std::string name, std::string value, std::type_info const &valueType, ChimeraTK::TimeStamp  timeStamp);
	UA_StatusCode mapSelfToNamespace();
};
#endif // MTCA_PROCESSSCALAR_H
