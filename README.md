ControlSystem-OPCUA_Adapter
===========================
[![License](https://img.shields.io/badge/license-LGPLv3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0.html)
[![DOI](https://rodare.hzdr.de/badge/DOI/10.14278/rodare.3257.svg)](https://doi.org/10.14278/rodare.3257)
![Supported Platforms][api-platforms]

[api-platforms]: https://img.shields.io/badge/platforms-linux%20-blue.svg "Supported Platforms"

Welcome to the ControlSystem-OPCUA_Adapter project. 
This project add an control system adapter to [ChimeraTK](https://github.com/ChimeraTK), that is based on the OPC UA stack [open62541](https://open62541.org/).

In order to use the control system adapter a dedicated map file is required. The name of the map file is expected to be _ApplicationName_\_mapping.xml, where _ApplicationName_ is the name of the ChiemraTK application.

If no map file is present a default server is created using port 16664. The map file allows to change general OPC UA settings and the process of mapping ChimarTK process variables to OPC UA nodes.
In principle all process variables are mapped in the OPC UA server. Node IDs and browse names are derived from the process variables register paths.
The mapping allows to extend the variable tree based on the tree created from mapping the ChimeraTK process variables.
Details can be found in the documentation of the [xml scheme file](xmlschema/opcua_mapfile.xsd). An [example map file](examples/example_mapping.xml), illustrated how to set general properties of the OPC UA server.

In principle it is possible to install open62541 via official packages, but it is recommended to install the [open62541 interface](https://github.com/ChimeraTK/open62541-interface) included in ChiemraTK. It might include patches in the future and freezes the open62541 version. Thus, it is guaranteed that the control system adapter works - independent of upstream changes of the open62541 stack.
     
A [list of autors](AUTHORS.md) is contained in this directory.
