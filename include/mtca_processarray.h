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
 * 
 */

#ifndef MTCA_PROCESSARRAY_H
#define MTCA_PROCESSARRAY_H

#include <ua_mapped_class.h>
#include <mtca_processvariable.h>

#include <iostream>
#include <vector>
#include <string>

#include <TimeStamp.h>


class mtca_processarray : ua_mapped_class
{
  
private:
	std::string   	name;
	std::string   	valueType;
	std::vector<int32_t> 	value;
	mtca4u::TimeStamp	timeStamp;
	
public:
	~mtca_processarray();
	mtca_processarray(UA_Server *server, UA_NodeId baseNodeId, std::string name, std::vector<int32_t> value, std::type_info const &valueType, mtca4u::TimeStamp timeStamp);
	
	void setName(std::string name);
	std::string getName();
	
	void setType(std::string valueType);
	std::string getType();
	
	void setValue(std::vector<int32_t> value);
	std::vector<int32_t> getValue();
  
	void setTimeStamp(mtca4u::TimeStamp timeStamp);
	mtca4u::TimeStamp getTimeStamp();
	
	
	void mtca_processvariable_globalinit(std::string name, std::vector<int32_t> value, std::type_info const &valueType, mtca4u::TimeStamp timeStamp);
	UA_StatusCode mapSelfToNamespace();
};

#endif // MTCA_PROCESSARRAY_H
