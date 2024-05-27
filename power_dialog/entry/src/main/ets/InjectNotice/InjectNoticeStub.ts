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

import rpc from '@ohos.rpc';
import { injectNoticeUtil } from './InjectNoticeUtil';

const TAG = "InjectNotice";
const getConnectId = (...args) => {
    return args.join('-');
};

export enum CmdCode {
    OPEN_NOTICE = 0,
    CLOSE_NOTICE_BY_REQUST = 1,
};

export class InjectNoticeStub extends rpc.RemoteObject {
    constructor(des) {
        console.debug(TAG, `InjectNoticeStub constructor start`);
        if (typeof des == 'string') {
            console.debug(TAG, `InjectNoticeStub constructor typeof string`);
            super(des)
        } else {
            console.debug(TAG, `InjectNoticeStub constructor typeof not string`);
            return
        }
    }

    onRemoteRequest(code, data, reply, option): boolean {
        console.debug(TAG, `onRemoteRequest start`);
        const connectId = getConnectId(rpc.IPCSkeleton.getCallingPid(), rpc.IPCSkeleton.getCallingTokenId());
        console.info(TAG, `onRemoteRequest start ${connectId}`);
        switch (code) {
            case CmdCode.OPEN_NOTICE: {
                console.debug(TAG, `RpcServer:sync open_notice is called`);
                let pid = data.readInt();
                console.debug(TAG, `code:${code} pid: ${pid}`);
                let ret: number = 0;
                let retStr: string = "success";
                try {
                    let bOk = injectNoticeUtil.sendNotice();
                    console.debug(TAG, `SendNotice() code:${code} pid: ${pid}`);
                } catch (e) {
                    ret = -1;
                    console.error(TAG, `send notice failed:${e}`);
                    retStr = "failed"
                }

                reply.writeInt(ret);
                reply.wtireString('retStr');
            }
                break;
            case CmdCode.CLOSE_NOTICE_BY_REQUST: {
                console.debug(TAG, `RpcServer:sync close_notice is called`);
                let pid = data.readInt();
                console.debug(TAG, `code:${code} pid: ${pid}`);
                reply.writeInt(0);
                reply.wtireString('success');
            }
                break;
            default:
                reply.writeInt(-1);
                reply.wtireString("not support");

        }
        return true;
    }
};