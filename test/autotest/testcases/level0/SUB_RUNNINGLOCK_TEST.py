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


class SUB_RUNNINGLOCK_TEST(TestCase):

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
        self.log.info("SUB_RUNNINGLOCK_TEST start")
        #处理可能会弹出的USB连接方式弹窗
        self.driver.touch(BY.text("确定"), EXCEPTION=False)


        Step("预置条件1：设置休眠时长30s")
        self.driver.Screen.set_sleep_time(30)

        #清除日志
        host.shell("hdc -t {} shell rm -r /data/log/hilog".format(self.sn))
        host.shell("hdc -t {} shell hilog -d /system/bin/samgr".format(self.sn))

        #回到桌面
        self.driver.go_home()

    def test_step(self):
        Step("启动华为视频应用")
        self.driver.start_app("com.huawei.hmsapp.himovie")
        self.driver.wait(0.5)

        #处理第一次弹出的同意使用app的选项
        self.driver.touch(BY.text("同意").type("Button"), EXCEPTION=False)
        self.driver.wait(0.5)


        #点击免费专区
        self.driver.touch(BY.key('PafSubTabBars.Text.text').text('免费专区'))

        #通过相对位置点击控件，开始播放视频
        self.driver.touch(BY.isAfter(BY.key('3560')).isBefore(BY.type('RelativeContainer')).key('ImageTextCard.Image.imageUrl').type('Image'))
        self.driver.wait(0.5)


        self.driver.wait(30)
        Step("检查是否为亮屏状态")
        self.driver.Screen.check_on()


    def teardown(self):
        Step("收尾工作")
        self.log.info("SUB_RUNNINGLOCK_TEST down")
        self.driver.stop_app("com.huawei.hmsapp.himovie")
        #回到桌面
        self.driver.go_home()