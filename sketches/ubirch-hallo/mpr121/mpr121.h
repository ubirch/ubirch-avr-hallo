/**
 * Definitions for the MPR121 capacitive sensor chip.
 *
 * data sheet: http://cache.freescale.com/files/sensors/doc/data_sheet/MPR121.pdf
 *
 * TODO: add GPIO functionality
 *
 * @author Matthias L. Jugel
 *
 * == LICENSE ==
 * Copyright 2015 ubirch GmbH (http://www.ubirch.com)
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

#ifndef UBIRCH_MPR121_H
#define UBIRCH_MPR121_H

#include <stdint.h>

#define MPR_DEVICE_ADDRESS  0x5A

// MPR 121 registers
#define MPR_R_TOUCHSTATUS   0x00
#define MPR_R_TOUCHSTATUS_F 0x04
#define MPR_R_BASELINE      0x1E
#define MPR_R_MHD_RISING    0x2B
#define MPR_R_NHD_RISING    0x2C
#define MPR_R_NCL_RISING    0x2D
#define MPR_R_FDL_RISING    0x2E
#define MPR_R_MHD_FALLING   0x2F
#define MPR_R_NHD_FALLING   0x30
#define MPR_R_NCL_FALLING   0x31
#define MPR_R_FDL_FALLING   0x32
#define MPR_R_NHD_TOUCHED   0x33
#define MPR_R_NCL_TOUCHED   0x34
#define MPR_R_FDL_TOUCHED   0x35

#define MPR_R_REL_THRESHOLD 0x41
#define MPR_R_TCH_THRESHOLD 0x42
#define MPR_R_DEBOUNCE      0x5B
#define MPR_R_CDC_CONFIG    0x5C
#define MPR_R_CDT_CONFIG    0x5D
#define MPR_R_ECR           0x5E

#define MPR_R_RESET         0x80

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Set up a value in a register on the sensor.
 * @param reg the register to write
 * @param data the value to write
 */
void mpr_set(uint8_t reg, uint8_t data);

/**
 * Get current setup from a register on the sensor.
 * @param reg the register to write
 */
uint8_t mpr_get(uint8_t reg);

/**
 * Reset the sensor.
 * A non-zero return indicates an error condition.
 */
uint8_t mpr_reset(void);

/**
 * Set threshold for a pin for touch and release.
 */
void mpr_set_threshold(uint8_t pin, uint8_t touch, uint8_t release);

/**
 * Get current status (touched or not) for all pins at once.
 */
uint16_t mpr_status(void);

/**
 * Get current status (touched or not) for the given pin in filtered form.
 */
uint16_t mpr_status_filtered(uint8_t pin);

/**
 * Get baseline for the given pin in filtered form.
 */
uint16_t mpr_baseline(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif //UBIRCH_MPR121_H
