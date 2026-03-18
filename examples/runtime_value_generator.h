// SPDX-FileCopyrightText: Helmholtz-Zentrum Dresden-Rossendorf, FWKE, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-FileCopyrightText: 2016 Chris Iatrou <Chris_Paul.Iatrou@tu-dresden.de>
// SPDX-FileCopyrightText: 2016 Julian Rahm <Julian.Rahm@tu-dresden.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <ChimeraTK/ControlSystemAdapter/DevicePVManager.h>

#include <thread>

using namespace ChimeraTK;

class runtime_value_generator {
 private:
  bool running;
  std::thread valueGeneratorThread;

 public:
  runtime_value_generator(boost::shared_ptr<DevicePVManager> devManager);
  ~runtime_value_generator();
  void generateValues(boost::shared_ptr<DevicePVManager> devManager);
};
