#
# Copyright 2020-2022 GoPro Inc.
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

import array
import random


# https://en.wikipedia.org/wiki/Color_term
class COLORS:
    # fmt: off
    white =   (1.0, 1.0, 1.0)
    black =   (0.0, 0.0, 0.0)
    red =     (1.0, 0.0, 0.0)
    orange =  (1.0, 0.5, 0.0)
    yellow =  (1.0, 1.0, 0.0)
    cgreen =  (0.5, 1.0, 0.0)  # Chartreuse green
    green =   (0.0, 1.0, 0.0)
    sgreen =  (0.0, 1.0, 0.5)  # Spring green
    cyan =    (0.0, 1.0, 1.0)
    azure =   (0.0, 0.5, 1.0)
    blue =    (0.0, 0.0, 1.0)
    violet =  (0.5, 0.0, 1.0)
    magenta = (1.0, 0.0, 1.0)
    rose =    (1.0, 0.0, 0.5)
    # fmt: on


def get_random_color_buffer(rng: random.Random, dim: int) -> array.array:
    data = []
    for _ in range(dim**2):
        data += (rng.random(), rng.random(), rng.random(), 1.0)
    return array.array("f", data)
