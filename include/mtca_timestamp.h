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

/*
 * Klasse mappt die ControlSystemAdapter TimeStamp auf den erstellten OPC UA TimeStamp
 */

#ifndef MTCA_TIMESTAMP_H
#define MTCA_TIMESTAMP_H

#include <ua_mapped_class.h>

#include <iostream>


class mtca_timestamp : ua_mapped_class {
	private:
		uint32_t seconds;
		uint32_t nanoseconds;
		uint32_t index0;
		uint32_t index1;

	public:
		
		mtca_timestamp(UA_Server *server, UA_NodeId baseNodeId, uint32_t seconds, uint32_t nanoseconds, uint32_t index0, uint32_t index1);
		mtca_timestamp();
		~mtca_timestamp();
		
		void setSeconds(uint32_t seconds);
		uint32_t getSeconds();
	
		void setNanoseconds(uint32_t nanoseconds);
		uint32_t getNanoseconds();
		
		void setIndex0(uint32_t index0);
		uint32_t getIndex0();
		
		void setIndex1(uint32_t index1);
		uint32_t getIndex1();	
	
		void mtca_timestamp_globalinit(uint32_t seconds, uint32_t nanoseconds, uint32_t index0, uint32_t index1);
	
		UA_StatusCode mapSelfToNamespace();
};

#endif // MTCA_TIMESTAMP_H
