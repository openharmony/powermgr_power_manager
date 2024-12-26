#!/usr/bin/env python3
#-*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import json
import os


def read_json_files_in_directory(path, type_device):
    """
    @func:Configuration file to be modified
    @param path: path name
    @param type_device:Types that need to be modified
    """
    if not os.path.exists(path):
        print(f"path:'{path}'not exist")
        return
    for root, dirs, files in os.walk(path):
        for filename in files:
            if filename.endswith('.json'):
                filepath = os.path.join(root, filename)
                with open(filepath, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                for item in data['environment']:
                    item['label'] = type_device
                data = json.dumps(data, indent=4, ensure_ascii=False)
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(data)


json_path = "testcases"

all_types = ["phone", "car", "tv", "watch", "tablet", "2in1"]
all_types_name = ["phone", "car", "tv", "watch", "tablet", "pc"]
print("Please enter the serial number of the device to be tested:1 2 3 4 5 6")
print("1:phone")
print("2:car")
print("3:tv")
print("4:watch")
print("5:tablet")
print("6:pc")
print("please input：")
type_number = int(input())
numbers = [1, 2, 3, 4, 5, 6]
if type_number not in numbers:
    print("Input error, please execute again")
else:
    type_device = all_types[type_number - 1]
    read_json_files_in_directory(json_path, type_device)
    print("Select Test " + all_types_name[type_number - 1] + " successfully!")
