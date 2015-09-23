/**
 * Simple checksum algorithm for the data transfer.
 *
 * See https://en.wikipedia.org/wiki/Fletcher%27s_checksum
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

#ifndef UBIRCH_CHECKSUM_H
#define UBIRCH_CHECKSUM_H

uint16_t fletcher16(uint8_t const *data, size_t bytes);

#endif //UBIRCH_CHECKSUM_H
