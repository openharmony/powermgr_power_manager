# Power Manager<a name="EN-US_TOPIC_0000001115047353"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section19472752217)
-   [Repositories Involved](#section63151229062)

## Introduction<a name="section11660541593"></a>

The Power Manager module provides the following functions:

1.  Rebooting the device
2.  Managing running locks
3.  Obtaining the power status

**Figure 1** Architecture of the Power Manager module<a name="fig106301571239"></a>

![](figures/power-management-subsystem-architecture.png "Architecture of the power management subsystem")

## Directory Structure<a name="section19472752217"></a>

```
/base/powermgr/power_manager
├── figures                  	# Architecture
├── frameworks                  # Framework layer
│   ├── napi                  	# NAPI layer
│   └── native                  # Native layer
├── interfaces                  # API layer
│   └── inner_api               # Internal APIs
├── power_dialog                # Power dialog
├── sa_profile                  # SA profile
└── services                    # Service layer
│   ├── native                  # Native layer
│   └── zidl                    # Zidl API layer
├── test                        # Test cases
│   ├── fuzztest                # Fuzz test
│   ├── unittest                # Unit test
│   ├── systemtest              # System test
│   └── utils                   # Test tools
└── utils                       # Utilities
```



## Repositories Involved<a name="section63151229062"></a>

[Power Management Subsystem](https://gitee.com/openharmony/docs/blob/master/en/readme/power-management.md)

**powermgr_power_manager**

[powermgr_display_manager](https://gitee.com/openharmony/powermgr_display_manager)

[powermgr_battery_manager](https://gitee.com/openharmony/powermgr_battery_manager)

[powermgr_thermal_manager](https://gitee.com/openharmony/powermgr_thermal_manager)

[powermgr_battery_statistics](https://gitee.com/openharmony/powermgr_battery_statistics)

[powermgr_battery_lite](https://gitee.com/openharmony/powermgr_battery_lite)

[powermgr_powermgr_lite](https://gitee.com/openharmony/powermgr_powermgr_lite)
