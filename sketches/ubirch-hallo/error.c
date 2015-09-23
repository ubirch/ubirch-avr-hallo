/**
 * ...
 *
 * @author Matthias L. Jugel
 *
 * Copyright 2015 ubirch GmbH (http://www.ubirch.com)
 *
 * == LICENSE ==
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

#include <Arduino.h>
#include "error.h"

void s() {
    enable_led();
    delay(500);
    disable_led();
    delay(500);
}

void l() {
    enable_led();
    delay(1500);
    disable_led();
    delay(500);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void error(uint8_t code) {
    for (;;) {
        switch (code) {
            case HALLO_ERROR_AUDIO:
                s(); s(); l();
                break;
            case HALLO_ERROR_SDCARD:
                s(); l(); l();
                break;
            default:
                l();
                break;
        }
        delay(2000);
    }
}

#pragma clang diagnostic pop
