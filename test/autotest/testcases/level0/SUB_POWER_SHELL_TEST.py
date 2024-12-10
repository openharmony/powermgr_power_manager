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


class SUB_POWER_SHELL_TEST(TestCase):

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
        self.log.info("SUB_POWER_SHELL_TEST start")
        #处理可能会弹出的USB连接方式弹窗
        self.driver.touch(BY.text("确定"), EXCEPTION=False)

        Step("预置条件1：设置休眠时长15s")
        self.driver.Screen.set_sleep_time(15)

        Step("预置条件2：关闭AOD息屏显示")
        self.driver.start_app("com.huawei.hmos.settings")

        #点击顶部搜索设置项
        self.driver.touch(BY.type('SearchField'))
        self.driver.wait(0.5)

        self.driver.wait(2)

        #处理可能会弹出的小艺使用请求
        self.driver.touch(BY.text("同意").type("Button"), EXCEPTION=False)
        self.driver.touch(BY.text("下一步").type("Button"), EXCEPTION=False)

        #输入xiping
        self.driver.input_text(BY.type("SearchField"), "xiping")

        #点击弹出的熄屏显示控件
        self.driver.touch(BY.text("桌面和个性化 > 熄屏显示设置"))
        self.driver.switch_component_status(BY.type('Toggle'), False)


        #回到桌面
        self.driver.go_home()

        #清除日志
        host.shell("hdc -t {} shell rm -r /data/log/hilog".format(self.sn))
        host.shell("hdc -t {} shell hilog -d /system/bin/samgr".format(self.sn))

    def test_step(self):
        Step("hdc shell 下执行power-shell --help")
        result = self.driver.System.execute_command("power-shell --help")

        CheckPoint("只打印setmode、wakeup、suspend、timeout、help命令，其他不打印，且命令功能正常")
        assert "setmode :    Set power mode" in result
        assert "wakeup  :    Wakeup system and turn screen on" in result
        assert "suspend :    Suspend system and turn screen off" in result
        assert "timeout :    Override or Restore screen off time" in result
        assert "help    :    Show this help menu" in result


        Step("设备执行pwoer-shell命令：power-shell suspend")
        self.driver.System.execute_command("power-shell suspend")

        CheckPoint("原本的power-shell命令可以正常使用")
        self.driver.Screen.check_on(expect_on=False)

        Step("设备执行pwoer-shell命令：power-shell wakeup")
        self.driver.System.execute_command("power-shell wakeup")
        self.driver.Screen.check_on()

        Step("设备执行pwoer-shell命令：power-shell timeout -o 5000")
        self.driver.System.execute_command("power-shell timeout -o 5000")
        time.sleep(5)
        self.driver.Screen.check_on(expect_on=False)

        Step("设备执行pwoer-shell命令：power-shell timeout -r")
        self.driver.System.execute_command("power-shell timeout -r")
        #恢复超时灭屏时间后唤醒设备
        self.driver.System.execute_command("power-shell wakeup")
        #解锁进入桌面
        self.driver.ScreenLock.unlock()
        time.sleep(15)
        #等待15s后灭屏
        self.driver.Screen.check_on(expect_on=False)


    def teardown(self):
        Step("收尾工作")
        self.log.info("SUB_POWER_SHELL_TEST down")

        #回到桌面
        self.driver.go_home()