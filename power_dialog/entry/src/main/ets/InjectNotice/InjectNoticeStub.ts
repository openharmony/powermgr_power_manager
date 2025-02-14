/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import Base from '@ohos.base';
import rpc from '@ohos.rpc';
import deviceInfo from '@ohos.deviceInfo';
import {NOTICE_ID, injectNoticeUtil } from './InjectNoticeUtil';
import {CapsuleUtil} from './CapsuleUtil';

const TAG = 'InjectNotice';
const getConnectId = (...args): string => {
    return args.join('-');
};
const INJECT_NOTICE_INTERFACE_TOKEN = 'ohos.multimodalinput.IInjectNotice';

export enum CmdCode {
    OPEN_NOTICE = 0,
    CLOSE_NOTICE_BY_REQUST = 1,
};

export class InjectNoticeStub extends rpc.RemoteObject {
    constructor(des) {
        console.debug(TAG, `InjectNoticeStub constructor start`);
        if (typeof des === 'string') {
            console.debug(TAG, `InjectNoticeStub constructor typeof string`);
            super(des);
        } else {
            console.debug(TAG, `InjectNoticeStub constructor typeof not string`);
            return;
        }
    }

    onRemoteRequest(code, data, reply, option): boolean {
        console.debug(TAG, `onRemoteRequest start deviceInfo.deviceType:${deviceInfo.deviceType}`);
        const connectId = getConnectId(rpc.IPCSkeleton.getCallingPid(), rpc.IPCSkeleton.getCallingTokenId());
        console.info(TAG, `onRemoteRequest start ${connectId}`);
        let token: string = data.readInterfaceToken();
        if (token !== INJECT_NOTICE_INTERFACE_TOKEN) {
            reply.writeInt(-1);
            reply.writeString('the token does not match');
            return true;
        }

        if (deviceInfo.deviceType === '2in1') {
            return this.handlePC(code, data, reply, option);
        }
        switch (code) {
            case CmdCode.OPEN_NOTICE: {
                console.debug(TAG, `RpcServer:open notice is called`);
                let pid = data.readInt();
                console.debug(TAG, `code:${code} pid: ${pid}`);
                let ret: number = 0;
                let retStr: string = 'success';
                try {
                    injectNoticeUtil.sendNotice();
                    console.debug(TAG, `SendNotice() code:${code} pid: ${pid}`);
                } catch (e) {
                    ret = -1;
                    console.error(TAG, `send notice failed:${e}`);
                    retStr = 'failed';
                }
                reply.writeInt(ret);
                reply.writeString(retStr);
            }
            break;
            case CmdCode.CLOSE_NOTICE_BY_REQUST: {
                console.debug(TAG, `RpcServer:close notice is called`);
                let pid = data.readInt();
                console.debug(TAG, `code:${code} pid: ${pid}`);
                injectNoticeUtil.cancelNotificationById(NOTICE_ID);
                reply.writeInt(0);
                reply.writeString('success');
            }
            break;
            default:
                reply.writeInt(-1);
                reply.writeString('not support');
        }
        console.debug(TAG, `onRemoteRequest end`);
        return true;
    }

    handlePC(code, data, reply, option): boolean {

        switch (code) {
            case CmdCode.OPEN_NOTICE: {
                console.debug(TAG, `RpcServer:open notice is called`);
                try {
                    console.debug(TAG, ` CapsuleUtil.getInstance beign:${deviceInfo.deviceType}`);
                    let instance: CapsuleUtil = CapsuleUtil.getInstance();
                    console.debug(TAG, ` processCapsulebeign:${deviceInfo.deviceType}`);
                    instance.processCapsule(true);
                } catch (error) {
                    let err = error as Base.BusinessError;
                    console.error(TAG, `CapsuleUtil.getInstance() err:${JSON.stringify(err)}`);
                }
                reply.writeInt(0);
                reply.writeString('success');
                return true;
            }
            break;
            case CmdCode.CLOSE_NOTICE_BY_REQUST: {
                console.debug(TAG, `RpcServer:close notice is called`);
                try {
                    console.debug(TAG, `capsuleUtil.getInstance beign close:${deviceInfo.deviceType}`);
                    let instance: CapsuleUtil = CapsuleUtil.getInstance();
                    instance.processCapsule(false);
                    instance.closePanel();
                    console.debug(TAG, `capsuleUtil cancelAuthorization begin:${deviceInfo.deviceType}`);
                    instance.cancelAuthorization();
                    console.debug(TAG, `capsuleUtil cancelAuthorization end:${deviceInfo.deviceType}`);
                } catch (error) {
                    let err = error as Base.BusinessError;
                    console.error(TAG, `CapsuleUtil.getInstance()  close err:${JSON.stringify(err)}`);
                }
                reply.writeInt(0);
                reply.writeString('success');
                return true;
            }
            break;
            default:
                reply.writeInt(-1);
                reply.writeString('not support');
        }
        console.debug(TAG, `onRemoteRequest end`);
        return true;
    }
};