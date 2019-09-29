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

#include "mgos_lolin_button.h"

#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_sys_config.h"

enum mgos_lolin_button_cmd {
  GET_SLAVE_STATUS = 0x01,
  RESET_SLAVE,
  CHANGE_I2C_ADDRESS,
  GET_KEY_VALUE,
  // There's no API for these, in case you need it, send 3 bytes to the chip:
  // CHANGE_* command followed by 16-bit LE value of time, in 1/100 of a second.
  //   uint8_t cmd[] = {CHANGE_KEY_A_LONG_PRESS_TIME, 0x2c, 1};
  //   return mgos_i2c_write(bus, 0x31, cmd, 3, true);
  // This sets long press/hold time for button a to 3 seconds (0x12c).
  CHANGE_KEY_A_LONG_PRESS_TIME,
  CHANGE_KEY_B_LONG_PRESS_TIME,
  CHANGE_KEY_A_DOUBLE_PRESS_INTERVAL,
  CHANGE_KEY_B_DOUBLE_PRESS_INTERVAL,
};

struct mgos_lolin_button_ctx {
  int i2c_bus;
  int i2c_addr;
  struct mgos_i2c *bus;
  mgos_timer_id timer_id;
  enum mgos_lolin_button_state last_state_a;
  enum mgos_lolin_button_state last_state_b;
};

static void report_change(enum mgos_lolin_button_event ev,
                          enum mgos_lolin_button_state last_state,
                          enum mgos_lolin_button_state state, int i2c_bus,
                          int i2c_addr) {
  if (state == last_state) return;
  if (state == MGOS_LOLIN_BUTTON_RELEASE) {
    return;
  }
  struct mgos_lolin_button_status ev_arg = {
      .i2c_bus = i2c_bus,
      .i2c_addr = i2c_addr,
      .state = state,
  };
  mgos_event_trigger(ev, &ev_arg);
}

static void mgos_lolin_button_timer_cb(void *arg) {
  struct mgos_lolin_button_ctx *ctx = (struct mgos_lolin_button_ctx *) arg;
  int r = mgos_i2c_read_reg_b(ctx->bus, ctx->i2c_addr, GET_KEY_VALUE);
  if (r < 0) return;
  enum mgos_lolin_button_state state_a = (enum mgos_lolin_button_state)(r >> 4);
  enum mgos_lolin_button_state state_b =
      (enum mgos_lolin_button_state)(r & 0xf);
  report_change(MGOS_EV_LOLIN_BUTTON_A, ctx->last_state_a, state_a,
                ctx->i2c_bus, ctx->i2c_addr);
  report_change(MGOS_EV_LOLIN_BUTTON_B, ctx->last_state_b, state_b,
                ctx->i2c_bus, ctx->i2c_addr);
  ctx->last_state_a = state_a;
  ctx->last_state_b = state_b;
}

bool mgos_lolin_button_init_cfg(const struct mgos_config_lolin_button *cfg) {
  if (!cfg->enable) return true;
  struct mgos_i2c *bus = mgos_i2c_get_bus(cfg->i2c_bus);
  if (bus == NULL) {
    LOG(LL_ERROR, ("invalid i2c_bus %d", cfg->i2c_bus));
    return false;
  }
  int r = mgos_i2c_read_reg_w(bus, cfg->i2c_addr, GET_SLAVE_STATUS);
  if (r < 0) {
    LOG(LL_ERROR,
        ("LOLIN button %d/%#x failed: %d", cfg->i2c_bus, cfg->i2c_addr, r));
  }
  struct mgos_lolin_button_ctx *ctx =
      (struct mgos_lolin_button_ctx *) calloc(1, sizeof(*ctx));
  if (ctx == NULL) return false;
  ctx->i2c_bus = cfg->i2c_bus;
  ctx->i2c_addr = cfg->i2c_addr;
  ctx->bus = bus;
  ctx->timer_id = mgos_set_timer(cfg->poll_interval_ms, MGOS_TIMER_REPEAT,
                                 mgos_lolin_button_timer_cb, ctx);
  if (ctx->timer_id == MGOS_INVALID_TIMER_ID) {
    free(ctx);
    return false;
  }
  return true;
}

bool mgos_lolin_button_init(void) {
  mgos_event_register_base(MGOS_EV_LOLIN_BUTTON_BASE, __FILE__);
  if (!mgos_sys_config_get_lolin_button_enable()) return true;
  return mgos_lolin_button_init_cfg(mgos_sys_config_get_lolin_button());
}
