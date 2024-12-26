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

from devicetest.core.test_case import TestCase, Step, CheckPoint, get_report_dir
from hypium import UiDriver
import time
from hypium import *
from hypium.action.os_hypium.device_logger import DeviceLogger
from hypium.action.host import host
from hypium.model import UiParam


class SUB_KILL_FOUNDATION_TEST(TestCase):

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
        self.log.info("SUB_KILL_FOUNDATION_TEST start")
        #处理可能会弹出的USB连接方式弹窗
        self.driver.touch(BY.text("确定"), EXCEPTION=False)

        Step("预置条件1：设置休眠时长1min")
        self.driver.Screen.set_sleep_time(60)

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
        Step("kill foundation进程")
        self.driver.System.execute_command("kill -9 `pidof foundation`")

        Step("等待7s")
        time.sleep(7)

        Step("7s后解锁进入桌面")
        self.driver.ScreenLock.unlock()

        Step("按键灭屏")
        self.driver.press_key(KeyCode.POWER)
        self.driver.wait(2)

        Step("检查是否为灭屏状态")
        self.driver.Screen.check_on(expect_on=False)

        time.sleep(2)

        Step("按键亮屏")
        self.driver.press_key(KeyCode.POWER)
        self.driver.wait(1)

        Step("检查是否为亮屏状态")
        self.driver.Screen.check_on()



    def teardown(self):
        Step("收尾工作")
        self.log.info("SUB_KILL_FOUNDATION_TEST down")
        self.driver.stop_app("com.huawei.hmos.settings")

        #回到桌面
        self.driver.go_home()