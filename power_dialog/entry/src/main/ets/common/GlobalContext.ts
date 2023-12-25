/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

// Construct a singleton object
export default class GlobalContext {
  private constructor() {}
  private static instance: GlobalContext;
  private _objects = new Map<string, Object>();
  
  public static getContext(): GlobalContext {
    if (!GlobalContext.instance) {
      console.log('make a singleton object');
      GlobalContext.instance = new GlobalContext();
    }
    console.log('finished ~');
    return GlobalContext.instance;
  }
  
  getObject(value: string): Object | undefined {
    console.log('get singleton object value');
    return this._objects.get(value);
  }
  
  setObject(key: string, objectClass: Object): void {
    console.log('set singleton object key and value');
    this._objects.set(key, objectClass);
  }
}