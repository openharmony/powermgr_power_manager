# 电源管理服务组件<a name="ZH-CN_TOPIC_0000001115047353"></a>

-   [简介](#section11660541593)
-   [目录](#section19472752217)
-   [相关仓](#section63151229062)

## 简介<a name="section11660541593"></a>

电源管理服务组件提供如下功能：

1.  重启系统。
2.  管理休眠运行锁。
3.  系统电源状态查询。

**图 1**  电源管理子系统架构图<a name="fig106301571239"></a>  
![](figures/power-management-subsystem-architecture.png "电源管理子系统架构图")

## 目录<a name="section19472752217"></a>

```
/base/powermgr/power_manager
├── interfaces                   # 接口层
│   ├── innerkits                # 内部接口
│   └── kits                     # 外部接口
├── sa_profile                   # SA配置文件
└── services                     # 服务层
│   ├── native                   # native层
│   └── zidl                     # zidl层
└── utils                        # 工具和通用层
```



## 相关仓<a name="section63151229062"></a>

电源管理子系统

powermgr_battery_manager

**powermgr_power_manager**

powermgr_display_manager
