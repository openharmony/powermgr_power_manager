# power\_manager<a name="EN-US_TOPIC_0000001115047353"></a>

- [Introduction](#section11660541593)
- [Directory Structure](#section19472752217)
- [Repositories Involved](#section63151229062)

## Introduction<a name="section11660541593"></a>

Power Manager provides the following functionalities:

1.  Rebooting the device
2.  Managing running locks
3.  Obtaining the power status

**Figure  1**  Power management subsystem architecture<a name="fig106301571239"></a>  
![](figures/power-management-subsystem-architecture.png "power-management-subsystem-architecture")

## Directory Structure<a name="section19472752217"></a>

```
/base/powermgr/power_manager
├── interfaces                   # APIs
│   ├── innerkits                # Internal APIs
│   └── kits                     # External APIs
├── sa_profile                   # SA profile
└── services                     # Services
│   ├── native                   # Native services
│   └── zidl                     # zidl APIs
└── utils                        # Utilities and common code
```

## Repositories Involved<a name="section63151229062"></a>

Power management subsystem

powermgr_battery_manager

**powermgr_power_manager**

powermgr_display_manager
