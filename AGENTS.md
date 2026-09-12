# AGENTS.md - OpenHarmony 电源管理服务（Power Manager）

## 1. 代码地图

本仓库实现 OpenHarmony 电源管理服务（Power Manager），SA ID 3301，进程名 `powermgr`，库 `libpowermgrservice.z.so`。核心职责是系统电源状态机管理、休眠/唤醒、亮灭屏、运行锁（RunningLock）管理、关机/重启、电源模式与电源档位策略。最重要的架构边界是**客户端 `PowerMgrClient` 通过 IPC 调用 `PowerMgrService`，客户端只做转发和连接管理，所有业务状态由服务端 `PowerStateMachine` 持有**。

### 非本项目维护的目录

以下目录属于其他子模块或工具产物，不属于本项目维护范围，修改时请跳过：

- `power_dialog/`：关机/重启弹窗 UI（独立 ArkTS 应用，由 UI 团队维护，仅做集成对接）
- `frameworks/cj/`：CJ 语言绑定（由 CJ 团队维护）
- `frameworks/ets/taihe/`：taihe FFI 生成产物（由 `taihe_ffi_gen` 工具生成，不要手改）

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。以下子模块可按需新建嵌套 AGENTS.md 以提供更具体的指引：

- `power_dialog/`：关机/重启弹窗独立 ArkTS 应用，可新建 `power_dialog/AGENTS.md` 聚焦弹窗 UI 时序、签名、ArkTS 工程结构与宿主 IPC 回包约定
- `tools/ohos-powerManager/`：`power-shell` CLI 独立 npm 包，可新建 `tools/ohos-powerManager/AGENTS.md` 聚焦 CLI 命令注册、npm 发布流程、`hidumper` 对接
- `utils/`：跨仓共享工具层（permission/param/setting/shell/appmgr/ffrt/hookmgr/lib_loader/intf_wrapper），被 `battery_manager` / `thermal_manager` 等下游仓引用，可新建 `utils/AGENTS.md` 聚焦跨仓工具变更影响评估
- `services/native/include/{hibernate,shutdown,suspend,ulsr}/`：分领域内部接口头文件子目录，可按需为每个领域新建嵌套 AGENTS.md 说明子流程约束（如 `ulsr/AGENTS.md` 聚焦 ULSR 插件版本耦合）
- `test/`：测试组织目录，可新建 `test/AGENTS.md` 聚焦 unittest/systemtest/fuzztest/apitest/autotest 五类测试的运行约定

### 关键区域

- `interfaces/inner_api/native/`：公共 C++ API 头文件，含 `PowerMgrClient` 入口、`RunningLock`、回调接口（`IPowerStateCallback` / `IPowerModeCallback` / `IRunningLockChangedCallback` / `IProximityController` / `IScreenOffPreCallback`）及 `power_state_machine_info.h` / `running_lock_info.h` / `power_mode_info.h` 数据模型。所有 C++ 消费者依赖此层，变更影响面最大。
- `interfaces/inner_api/native/{hibernate,shutdown,suspend,ulsr}/`：分领域的内部接口头文件，对应 S3/S4/关机/接管挂起等子流程。
- `frameworks/napi/{power,runninglock,utils}/`：ArkTS NAPI 绑定，供 ArkTS 应用使用。
- `frameworks/native/shutdown/`：客户端关机流程实现。
- `services/zidl/`：IPC 接口实现（**本项目不使用 `.idl` 文件**，直接用 `*_stub.h/.cpp` + `*_proxy.h/.cpp` 手写 IPC）。
- `services/zidl/include/shutdown/`：关机相关回调接口（`TakeoverShutdownCallback` / `SyncShutdownCallback` / `AsyncShutdownCallback`）。
- `services/native/include/power_mgr_service.h`：服务端核心类（2000+ 行），管理 `PowerStateMachine` 和全部业务流程。
- `services/native/include/power_state_machine.h`：电源状态机（INACTIVE / ACTIVE / SLEEP / SHUTDOWN），修改前必须理解透状态转移规则。
- `services/native/include/power_mode_module.h` / `power_mode_policy.h` / `power_save_mode.h`：电源模式与省电策略。
- `services/native/include/actions/`：电源动作执行（亮屏/灭屏/休眠/唤醒）。
- `services/native/include/adapter/`：HDI 适配层（power_proxy、input、display、light 等）。
- `services/native/src/`：服务端业务实现，按子目录组织。
- `services/native/profile/`：默认电源模式配置文件。
- `utils/native/`、`utils/permission/`、`utils/param/`、`utils/setting/`、`utils/shell/`、`utils/vibrator/`、`utils/appmgr/`、`utils/ability/`、`utils/ffrt/`、`utils/hookmgr/`、`utils/lib_loader/`、`utils/intf_wrapper/`：跨仓共享的工具模块（`utils/` 路径由本仓和 `battery_manager`/`thermal_manager` 共同使用）。
- `etc/init/`、`etc/para/`：系统启动配置和系统参数。
- `power_dialog/`：关机/重启弹窗 ArkTS 应用。
- `tools/ohos-powerManager/`：`power-shell` 命令行工具。
- `sa_profile/3301.json`：SA 3301 注册配置。
- `powermgr.gni`：30+ 个特性开关（`power_manager_feature_*`）。

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 公共 API 变更 | `interfaces/inner_api/native/include/` 头文件 -> `frameworks/native/` 实现 -> `frameworks/napi/` ArkTS 绑定 -> 对应 `*.map` 版本脚本 |
| IPC 接口变更 | `services/zidl/include/*.h`（接口定义）+ `services/zidl/src/*.cpp`（proxy/stub 实现）+ `interfaces/inner_api/native/include/*_ipc_interface_code.h`（IPC 码枚举） |
| 电源状态机 | `services/native/include/power_state_machine.h` + `services/native/src/power_state_machine.cpp` |
| 运行锁 | `interfaces/inner_api/native/include/running_lock.h` + `running_lock_info.h` + `services/native/src/running_lock_*` |
| 亮灭屏流程 | `services/native/include/actions/` + `services/native/include/power_mgr_service.h` |
| 关机/重启 | `services/zidl/include/shutdown/` + `frameworks/native/shutdown/` + `power_dialog/` |
| 休眠/唤醒接管（ULSR） | `interfaces/inner_api/native/include/ulsr/` + `services/zidl/include/ulsr_callback_*.h` |
| 电源模式 | `services/native/include/power_mode_module.h` + `power_mode_policy.h` + `services/native/profile/` |
| 距离传感器 | `interfaces/inner_api/native/include/iproximity_controller.h` + `services/native/src/proximity_controller*.cpp` |
| 特性开关 | `powermgr.gni` `declare_args()` 段 + 对应 `defines` |
| 新增/修改测试 | `test/unittest/`（单元）+ `test/systemtest/`（系统）+ `test/fuzztest/`（模糊，40+ 个）+ `test/apitest/`（接口）+ `test/autotest/`（自动化） |
| 构建配置 | `bundle.json` + `powermgr.gni` + 子目录 `BUILD.gn` |

### 架构分层

```
应用层
  ├─ ArkTS 应用 -> frameworks/napi/{power,runninglock,utils} (NAPI 绑定)
  │                └─ frameworks/ets/taihe (taihe FFI 生成)
  ├─ CJ 应用 -> frameworks/cj (CJ 绑定)
  └─ C++ 应用/系统组件 -> interfaces/inner_api/native (完整 C++ API)
          ↓
客户端 SDK
  PowerMgrClient (frameworks/native, 公共入口)
    -> IPowerMgrAsync proxy (services/zidl, IPC 客户端)
        ↓ IPC (SystemAbility 3301)
服务端
  PowerMgrService (services/native, 继承 SystemAbility)
    -> PowerStateMachine (状态机：INACTIVE / ACTIVE / SLEEP / SHUTDOWN)
      ├─ actions/ (亮屏/灭屏/休眠/唤醒动作)
      ├─ PowerModeModule + PowerModePolicy + PowerSaveMode (模式策略)
      ├─ RunningLock 管理 (runninglock)
      ├─ ProximityController (距离传感器)
      ├─ adapter/ (HDI 适配：power_proxy、display、input、light)
      ├─ 关机流程 -> shutdown callbacks (services/zidl/include/shutdown)
      │              └─ power_dialog/ (ArkTS 关机弹窗 UI)
      └─ ULSR (运行锁接管插件) -> ulsr_callback_*
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的代码路径和文档。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 公共 API 新增/修改 | `interfaces/inner_api/native/include/*.h` + `frameworks/native/` 实现 + `frameworks/napi/` ArkTS 绑定 + `*.map` 版本脚本 + `interfaces/inner_api/native/include/*_ipc_interface_code.h`（IPC 码） |
| IPC 接口变更 | `services/zidl/include/*.h` + `services/zidl/src/*.cpp` + 对应回调头文件（**本项目不使用 `.idl`，直接手写 proxy/stub**） |
| 状态机变更 | `services/native/include/power_state_machine.h` + `services/native/src/power_state_machine.cpp` + `interfaces/inner_api/native/include/power_state_machine_info.h` |
| 运行锁变更 | `interfaces/inner_api/native/include/running_lock.h` + `running_lock_info.h` + `interfaces/inner_api/native/include/irunning_lock_changed_callback.h` + `services/native/src/running_lock_*` |
| 关机/重启流程 | `frameworks/native/shutdown/` + `services/zidl/include/shutdown/` + `power_dialog/` + `etc/init/` 启动脚本 |
| 休眠接管（S4 / takeover_suspend） | `interfaces/inner_api/native/include/{hibernate,suspend}/` + `services/zidl/include/sync_{sleep,hibernate}_callback_*.h` + `services/zidl/include/takeover_suspend_callback_*.h` |
| ULSR 插件 | `interfaces/inner_api/native/include/ulsr/` + `services/zidl/include/ulsr_callback_*.h` + `powermgr.gni` 中 `power_manager_feature_enable_ulsr_plugin` |
| 电源模式 | `services/native/include/power_mode_module.h` + `power_mode_policy.h` + `power_save_mode.h` + `services/native/profile/`（默认模式配置） |
| 距离传感器逻辑 | `interfaces/inner_api/native/include/iproximity_controller.h` + `services/native/src/proximity_controller*.cpp` + `powermgr.gni` 中 `power_manager_feature_proximity_controller_override` |
| 屏幕事件回调 | `interfaces/inner_api/native/include/iscreen_common_event_controller.h` + `interfaces/inner_api/native/include/iscreen_off_pre_callback.h` |
| 双击/拾起/运动唤醒 | `powermgr.gni` 中 `power_manager_feature_{doubleclick,pickup,movement}` |
| 关机充电模式 | `powermgr.gni` 中 `power_manager_feature_poweroff_charge` + `power_manager_feature_charging_type_setting` |
| 权限校验 | `utils/permission/` + 调用 `VerifyAccessToken` 的位置（grep `AccessTokenKit`） |
| 持久化参数 | `utils/param/` + `etc/para/` |
| 新增特性 | `powermgr.gni` 添加 `declare_args()` -> 对应 `defines` -> 代码用 `#ifdef` 包裹 |
| 新增/修改测试 | `test/unittest/` + `test/systemtest/` + `test/fuzztest/` + `test/apitest/inner_api/` + `test/autotest/testcases/` + 对应 `BUILD.gn` |
| 构建配置 | `bundle.json`（依赖、syscap、rom/ram）+ `powermgr.gni`（特性开关）+ 各 `BUILD.gn` |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `interfaces/inner_api/native/` | 所有 C++ 消费者的公共 API，变更需同步 `frameworks/napi/`、`frameworks/cj/`、`frameworks/ets/taihe/` 三套绑定，并检查 `*.map` 版本脚本 |
| `services/zidl/` | IPC 接口层，**手写 proxy/stub**（不通过 `.idl` 生成），修改接口签名需同时改 `_stub`/`_proxy` 两套文件并同步 `_ipc_interface_code.h` 码值 |
| `services/native/include/power_mgr_service.h` | 服务端核心类（2000+ 行），修改前需理解状态机/运行锁/亮灭屏/关机四大主流程 |
| `services/native/include/power_state_machine.h` | 状态机核心，所有状态转移入口，修改转移规则影响全系统行为 |
| `services/native/include/power_mode_*.h` | 模式策略链，`PowerModeModule` 入口 -> `PowerModePolicy` 仲裁 -> `PowerSaveMode` 持久化 |
| `services/native/include/actions/` | 动作执行器，亮屏/灭屏动作会调用 `display_manager` 和 HDI |
| `services/native/include/adapter/` | HDI 适配层，`power_proxy_1.3.z.so` 等版本依赖见 `sa_profile/3301.json` 的 `min_hdi_proxy_version` |
| `power_dialog/` | 独立 ArkTS 应用，仅在新增/修改关机弹窗 UI 时进入 |
| `frameworks/ets/taihe/` | taihe FFI **生成产物**，修改 `.taihe` 源文件后由构建系统重生成，不要直接编辑 |
| `powermgr.gni` | 30+ 个特性开关，每个开关影响编译范围，新增开关需同时更新 `bundle.json` 的 `features` 列表 |
| `utils/` | 跨仓共享工具（permission/param/setting/shell/appmgr/ffrt/hookmgr/lib_loader/intf_wrapper），修改影响 `battery_manager`/`thermal_manager` 等下游仓 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| RunningLock | 运行锁，阻止系统进入休眠。修改锁生命周期/类型/代理逻辑影响所有持锁业务 | `interfaces/inner_api/native/include/running_lock.h` + `running_lock_info.h` + `services/native/src/running_lock_*` |
| PowerState | 电源状态（INACTIVE/ACTIVE/SLEEP/SHUTDOWN），状态机转移影响全系统 | `services/native/include/power_state_machine.h` + `interfaces/inner_api/native/include/power_state_machine_info.h` |
| S4 | 深度休眠（hibernate to RAM），特性开关默认关闭，启用需评估内存/磁盘兼容 | `powermgr.gni` 中 `power_manager_feature_enable_s4` + `interfaces/inner_api/native/include/hibernate/` |
| ULSR | Ultra-Latency Suspend Resume，低时延唤醒接管插件框架 | `interfaces/inner_api/native/include/ulsr/` + `services/zidl/include/ulsr_callback_*.h` |
| Takeover | 接管关机/挂起流程，允许上层应用拦截系统关机/挂起决策 | `services/zidl/include/{takeover_suspend_callback_*,shutdown/takeover_shutdown_callback_*}` |
| SyncShutdown / AsyncShutdown | 同步/异步关机回调，决定关机前是否阻塞等待业务回包 | `services/zidl/include/shutdown/{sync,async}_shutdown_callback_*.h` |
| SyncSleep / SyncHibernate | 同步睡眠/休眠回调 | `services/zidl/include/sync_{sleep,hibernate}_callback_*.h` |
| ScreenOffPre | 灭屏前回调，允许业务在灭屏前完成清理 | `interfaces/inner_api/native/include/iscreen_off_pre_callback.h` |
| ProximityController | 距离传感器控制器（通话灭屏），有 override 特性 | `interfaces/inner_api/native/include/iproximity_controller.h` |
| PowerMode | 电源模式（默认/省电/性能等），档位变更广播给所有订阅者 | `services/native/include/power_mode_module.h` + `interfaces/inner_api/native/include/power_mode_info.h` |
| SystemAbility / SA | OpenHarmony 系统服务框架，本服务 SA ID 3301 | `sa_profile/3301.json` |
| sptr | OpenHarmony 共享指针（`refbase.h`），非 `std::shared_ptr` | 全项目 |
| ErrCode | 错误码返回类型 | `interfaces/inner_api/native/include/power_errors.h` |
| FFRT | Function Flow Runtime，华为协作式异步任务调度框架 | `utils/ffrt/` |
| HookMgr | 钩子管理器，允许在固定执行点插入预处理逻辑 | `utils/hookmgr/` |
| HWTEST_F | OpenHarmony 测试用例宏 | 全项目测试代码 |
| 特性开关 | `powermgr.gni` 中 `declare_args()` 控制条件编译 | `powermgr.gni` |
| min_hdi_proxy_version | `sa_profile/3301.json` 中声明的 HDI 代理版本依赖（当前 `libpower_proxy_1.3.z.so`），降级 HDI 会破坏兼容性 | `sa_profile/3301.json` |
| powermgr 进程 | 本仓与 battery_manager/thermal_manager/display_manager 共享同一 `powermgr` 进程名，多 SA 共进程 | `sa_profile/3301.json` + 各仓 `sa_profile` |
| 双击/拾起唤醒 | `power_manager_feature_{doubleclick,pickup,movement}`，依赖传感器事件 | `powermgr.gni` 对应特性 |
| 关机充电 | 关机状态下进入充电模式，UI 由 `charger`（在 battery_manager 仓）负责 | `powermgr.gni` 中 `power_manager_feature_poweroff_charge` |

在计划阶段，必须声明：
- **任务分类**（如：公共 API 变更 / 状态机变更 / 运行锁变更 / 关机流程变更 / 新增特性 / 测试修改）
- **已读取的代码路径和文档**（具体到文件路径）
- **发现的约束**（架构不变量、禁止事项、特性开关依赖、HDI 版本依赖、跨仓依赖）
- **是否需要同步修改其他层**：
  - 公共 API 变更 -> 同步 NAPI / CJ / taihe 三套绑定 + `*.map` 版本脚本 + IPC 码枚举
  - `services/zidl/` 接口变更 -> 同步 stub + proxy + `_ipc_interface_code.h`
  - 特性开关变更 -> 同步 `bundle.json` 的 `features` 列表
  - `utils/` 变更 -> 检查 `battery_manager` / `thermal_manager` 是否受影响

## 3. 约束边界

### 架构不变量

- **客户端不持有业务状态**：`PowerMgrClient` 仅做 IPC 转发，所有状态由 `PowerMgrService` + `PowerStateMachine` 持有
- **状态机单一入口**：所有电源状态变更必须经 `PowerStateMachine` 的转移方法，禁止业务直接修改状态字段
- **运行锁生命周期由服务端管理**：客户端只能 `Create` / `Release`，锁的实际生效与代理由服务端仲裁
- **公共 API 表达稳定能力意图，不暴露内部实现细节**（如不暴露状态机字段、HDI 句柄）
- **权限校验在服务端入口完成**：所有写操作（`SuspendDevice` / `WakeupDevice` / `Shutdown` / `Reboot` / `SetDeviceMode`）必须有权限校验，不能依赖客户端
- **HDI 调用必须经 adapter 层**，禁止业务直接调用 HDI 接口
- **关机流程必须支持 takeover**：上层应用可拦截关机决策，禁止绕过 takeover 直接执行关机
- **DFX（日志、HiSysEvent、HiCollie、错误码）必须观测所有状态变更和 HDI 调用**
- **跨进程回调必须用 Parcelable 序列化**，所有回调接口继承 `IRemoteBroker`
- **IPC 调用必须设置死亡通知**，防止客户端异常导致服务端泄露

### 禁止事项

- **不要修改公共 API 签名、错误码、权限行为或生命周期语义**，除非任务明确要求；修改 `*.map` 中已有符号的可见性属于破坏性变更
- **不要直接编辑 `frameworks/ets/taihe/` 下的生成文件**，应修改 `.taihe` 源文件后由构建系统重生成
- **不要为通过测试而删除日志、HiSysEvent 事件、错误码或诊断信息**
- **不要绕过现有的 DFX、安全、兼容性检查**
- **不要直接修改 HDI 接口**：本仓是 HDI 消费方，HDI 接口由 `drivers_interface_power` 等仓维护
- **不要降级 `sa_profile/3301.json` 中的 `min_hdi_proxy_version`**：降级 HDI 版本会破坏与旧设备的兼容性
- **不要在 `services/zidl/` 中只改 stub 不改 proxy**（或反之）：两套必须同步
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审
- **不要在 `powermgr` 进程中执行长耗时同步操作**：本进程与 battery/thermal/display 共享，阻塞会拖死整个电源子系统
- **不要在关机/休眠路径中分配新内存或调用阻塞 IO**：路径在低内存/低 IO 能力下执行
- **不要绕过 `PowerStateMachine` 直接修改电源状态字段**
- **不要在 `utils/` 中加入 power_manager 私有业务**：`utils/` 是跨仓共享层，仅供工具能力

### 需确认后再修改

- **公共 API 签名变更**（需确认兼容性影响和版本策略，更新 `*.map` 版本脚本）
- **IPC 码值变更**（`*_ipc_interface_code.h`，需确认所有调用方同步更新，旧客户端可能因码值不匹配崩溃）
- **`sa_profile/3301.json` 中 `min_hdi_proxy_version` 调整**（需确认 HDI 团队和设备兼容性）
- **`powermgr.gni` 特性开关默认值翻转**（需评估对产品形态的影响，特别是 `power_manager_feature_enable_s4` / `power_manager_feature_poweroff_charge` / `power_manager_feature_runninglock`）
- **状态机新增状态或转移规则**（需评估所有依赖状态的业务：UI、应用框架、系统服务）
- **关机/休眠路径新增同步阻塞调用**（需评估关机超时风险）
- **`utils/` 跨仓工具的接口变更**（需确认 `battery_manager` / `thermal_manager` 是否受影响）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）
- **`power_dialog/` 弹窗 UI 行为变更**（需确认 UX 团队评审）

### 项目特定陷阱

- **IPC 码值稳定性**：`*_ipc_interface_code.h` 中的码值是 ABI 契约，新增接口必须追加新码值，不能复用或调整已有码值，否则旧客户端会调用错误接口
- **手写 proxy/stub 同步**：本项目不使用 `.idl`，修改接口签名必须同时改 stub（服务端入参解码）和 proxy（客户端出参编码），漏改一侧会导致 IPC 数据错乱
- **特性开关的 `defines` 双写**：`powermgr.gni` 中开关既要改 `declare_args()` 默认值，也要在对应 `if` 块更新 `defines`，漏掉一处会导致特性开关失效
- **`powermgr` 多 SA 共进程**：SA 3301/3302/3303/3308 在同一 `powermgr` 进程，一个 SA 崩溃会拖死全部，新增线程/锁需评估跨 SA 影响
- **关机流程的弹窗时序**：`power_dialog` 是独立 ArkTS 应用，关机决策必须等弹窗应用回包，超时机制在 `PowerMgrService` 中实现，不能改为同步阻塞
- **`ULSR` 插件的版本耦合**：ULSR 插件接口和宿主接口有版本绑定，新增/修改插件需同时更新版本号
- **`TakeoverShutdown` 的死锁风险**：takeover 回调中不能再触发关机决策，否则死锁
- **状态机转移的不可逆性**：`SHUTDOWN` 状态不可逆，进入后不能回退到 `ACTIVE`
- **`FFRT` 任务的生命周期**：`utils/ffrt/` 中的任务必须显式 wait 或 detach，泄漏会阻塞状态机

## 4. 验证闭环

### 最小验证

```bash
# 构建 power_manager 子系统（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target power_manager

# 构建全部测试
./build.sh --product-name rk3568 --build-target power_manager_test
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| 公共 API 变更 | `./build.sh --product-name rk3568 --build-target power_manager` + 同步构建所有依赖本仓的下游仓（`display_manager` / `thermal_manager` / `battery_statistics`） |
| IPC 接口变更 | `./build.sh --product-name rk3568 --build-target power_manager_unit_test` + 验证 stub/proxy 配对完整 |
| 状态机变更 | 跑 `test/unittest/power_state_machine_test` 全部用例 + `test/systemtest/` 关机/休眠/唤醒场景 |
| 运行锁变更 | 跑 `test/unittest/running_lock_test` + `test/fuzztest/createrunninglock_fuzzer` / `releaserunninglock_fuzzer` / `proxyrunninglock*_fuzzer` |
| 关机/重启变更 | 跑 `test/systemtest/` 关机相关用例 + `test/fuzztest/{shutdowndevice,rebootdevice,asyncshutdowncallback,takeovershutdowncallback}_fuzzer` |
| 休眠/唤醒 | `test/fuzztest/{suspenddevice,wakeupdevice,hibernate,forcesuspenddevice}_fuzzer` |
| 特性开关翻转 | 重新构建 `power_manager` 全量 + 验证 `bundle.json` 中 `features` 同步更新 |
| HDI 适配层变更 | 验证 `sa_profile/3301.json` 的 `min_hdi_proxy_version` 未被降级 |

### Done 定义

- 构建通过（子系统 + 单元测试 + 模糊测试 + CLI 工具 + 弹窗应用）
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- IPC 接口变更已同步 stub/proxy/IPC 码三处
- 特性开关变更已同步 `powermgr.gni` + `bundle.json`
- 公共 API 变更已同步 NAPI / CJ / taihe / `*.map` 版本脚本
- 涉及关机/休眠路径的变更已评估超时与死锁风险

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 是否触发跨层同步修改（NAPI / CJ / taihe / `*.map` / IPC 码 / `bundle.json` / `utils/`）
4. 是否影响特性开关默认值或 HDI 版本依赖
5. 是否触及架构不变量或需确认事项

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及关机/休眠/状态机的变更，必须人工复核代码逻辑并说明无法在沙箱验证的限制。
