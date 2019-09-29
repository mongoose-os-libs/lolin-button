/*
 * Copyright (c) 2019 Deomid "rojer" Ryabkov
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>

#include "mgos_event.h"

#define MGOS_EV_LOLIN_BUTTON_BASE MGOS_EVENT_BASE('L', 'B', 'T')
enum mgos_lolin_button_event {
  MGOS_EV_LOLIN_BUTTON_A =
      MGOS_EV_LOLIN_BUTTON_BASE, /* ev_data: struct mgos_lolin_button_status */
  MGOS_EV_LOLIN_BUTTON_B,
};

enum mgos_lolin_button_state {
  MGOS_LOLIN_BUTTON_RELEASE = 0,
  MGOS_LOLIN_BUTTON_PRESS,
  MGOS_LOLIN_BUTTON_LONG_PRESS,
  MGOS_LOLIN_BUTTON_DOUBLE_PRESS,
  MGOS_LOLIN_BUTTON_HOLD,
};

struct mgos_lolin_button_status {
  int i2c_bus;
  int i2c_addr;
  enum mgos_lolin_button_state state;
};
