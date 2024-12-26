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

from devicetest.utils.file_util import get_resource_path
from devicetest.core.test_case import TestCase, Step, CheckPoint, get_report_dir
from hypium import UiDriver
import time
from hypium import *
from hypium.action.os_hypium.device_logger import DeviceLogger
from hypium.action.host import host
from hypium.model import UiParam


class SUB_POWER_SHELL_SETMODE_TEST(TestCase):

    def __init__(self, configs):
        self.TAG = self.__class__.__name__
        TestCase.__init__(self, self.TAG, configs)
        self.tests = [
            "test_step"
        ]
        self.driver = UiDriver(self.device1)
        self.driver_width, self.driver_height = self.driver.get_display_size()
        self.sn = self.device1.device_sn

    def setup(self):
        self.log.info("SUB_POWER_SHELL_SETMODE_TEST start")


        #回到桌面
        self.driver.go_home()

        #清除日志
        host.shell("hdc -t {} shell rm -r /data/log/hilog".format(self.sn))
        host.shell("hdc -t {} shell hilog -d /system/bin/samgr".format(self.sn))

    def test_step(self):
        Step("hdc shell 下执行power-shell setmode 601")
        result = self.driver.System.execute_command("power-shell setmode 601")
        CheckPoint("power-shell命令是否可以正常使用")
        assert "Set Mode: 601" in result
        assert "Set Mode Success!" in result


        Step("hdc shell 下执行power-shell setmode 602")
        result = self.driver.System.execute_command("power-shell setmode 602")
        CheckPoint("power-shell命令是否可以正常使用")
        assert "Set Mode: 602" in result
        assert "Set Mode Success!" in result

        Step("hdc shell 下执行power-shell setmode 601")
        result = self.driver.System.execute_command("power-shell setmode 603")
        CheckPoint("power-shell命令是否可以正常使用")
        assert "Set Mode: 603" in result
        assert "Set Mode Success!" in result

        Step("hdc shell 下执行power-shell setmode 600")
        result = self.driver.System.execute_command("power-shell setmode 600")
        CheckPoint("power-shell命令是否可以正常使用")
        assert "Set Mode: 600" in result
        assert "Set Mode Success!" in result


    def teardown(self):
        Step("收尾工作")
        self.log.info("SUB_POWER_SHELL_SETMODE_TEST down")

        #回到桌面
        self.driver.go_home()