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


class SUB_RUNNINGLOCK_TIMEOUT_TEST(TestCase):

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
        self.log.info("SUB_RUNNINGLOCK_TIMEOUT_TEST start")

        #清除日志
        host.shell("hdc -t {} shell rm -r /data/log/hilog".format(self.sn))
        host.shell("hdc -t {} shell hilog -d /system/bin/samgr".format(self.sn))
        self.driver.install_app(package_path=power_hap_path)
        self.driver.ScreenLock.unLock()

        #回到桌面
        self.driver.go_home()

    def test_step(self):
        device_logger = DeviceLogger(self.driver).set_filter_string("powermgr")
        device_logger.start_log(get_report_dir() + '//runninglock_timeout.txt')

        Step("点击运行锁")
        self.driver.start_app(package_name="ohos.power.powermanager")
        # 通过相对位置点击控件
        self.driver.touch(BY.isAfter(BY.text('运行锁相关功能')).isBefore(BY.text('耗电统计')).type('Image'))

        self.driver.wait(0.5)

        # 输入超时时间10000
        self.driver.input_text(BY.type('TextInput'), '10000')
        self.driver.wait(0.5)

        self.driver.touch(BY.type('Image').isBefore(BY.key('keyboardRightButton')))
        self.driver.wait(0.5)

        self.driver.touch(BY.type('Button').text('持锁'))
        self.driver.wait(10)
        device_logger.stop_log()

        log_check_result = device_logger.check_log("try Lock, name: power_sample_timeout", EXCEPTION=True)
        self.driver.Assert.equal(log_check_result, True)

        log_check_result = device_logger.check_log("try UnLock, name: power_sample_timeout", EXCEPTION=True)
        self.driver.Assert.equal(log_check_result, True)
    

    def teardown(self):
        Step("收尾工作")
        self.log.info("SUB_RUNNINGLOCK_TIMEOUT_TEST down")
        self.driver.stop_app(package_name="ohos.power.powermanager")
        #回到桌面
        self.driver.go_home()