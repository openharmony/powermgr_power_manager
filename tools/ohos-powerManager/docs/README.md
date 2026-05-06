# ohos-powerManager CLI

## Overview

ohos-powerManager is a command-line interface tool for HarmonyOS/OHOS power management. It provides system-level control over device power state, including suspend, wakeup, power mode switching, and screen off timeout management.

## Dependencies

- PowerMgrClient (inner_api)
- nlohmann/json (via cJSON:cjson)
- c_utils
- hilog
- ipc_core

## Required Permissions

All commands require system caller identity and one of the following permissions:

| Permission | Required by |
|---|---|
| `ohos.permission.POWER_MANAGER` | suspend, wakeup, override-screen-off-time, restore-screen-off-time |
| `ohos.permission.POWER_OPTIMIZATION` | set-power-mode |

## Command List

| Command | Description | Parameters | Permission | Prerequisites |
|---|---|---|---|---|
| `suspend` | Suspend device and turn screen off | `[--immediately]` | POWER_MANAGER | None |
| `wakeup` | Wake up device and turn screen on | `[--detail <string>]` | POWER_MANAGER | None |
| `set-power-mode` | Set device power mode | `--mode <mode>` (normal/performance) | POWER_OPTIMIZATION | None |
| `override-screen-off-time` | Override screen off timeout | `--time <timeout_ms>` (positive integer) | POWER_MANAGER | None |
| `restore-screen-off-time` | Restore screen off timeout to default | (none) | POWER_MANAGER | Prior override recommended |

## Usage Examples

### Global Help

```bash
ohos-powerManager --help
```

### Suspend Device

```bash
# Suspend with default behavior
ohos-powerManager suspend

# Suspend immediately
ohos-powerManager suspend --immediately
```

Success output:
```json
{"type":"result","status":"success","data":{"command":"suspend","reason":"application","immediately":false}}
```

### Wakeup Device

```bash
# Wakeup with default detail
ohos-powerManager wakeup

# Wakeup with custom detail
ohos-powerManager wakeup --detail "user pressed power key"
```

Success output:
```json
{"type":"result","status":"success","data":{"command":"wakeup","reason":"application","detail":"cli-call"}}
```

### Set Power Mode

```bash
# Set normal mode
ohos-powerManager set-power-mode --mode normal

# Set performance mode
ohos-powerManager set-power-mode --mode performance
```

Success output:
```json
{"type":"result","status":"success","data":{"command":"set-power-mode","mode":"normal"}}
```

### Override Screen Off Time

```bash
# Override to 30 seconds
ohos-powerManager override-screen-off-time --time 30000

# Override to 5 minutes
ohos-powerManager override-screen-off-time --time 300000
```

Success output:
```json
{"type":"result","status":"success","data":{"command":"override-screen-off-time","timeout":30000}}
```

### Restore Screen Off Time

```bash
ohos-powerManager restore-screen-off-time
```

Success output:
```json
{"type":"result","status":"success","data":{"command":"restore-screen-off-time"}}
```

## Error Output Format

All errors are output as single-line JSON to stdout:

```json
{"type":"result","status":"failed","errCode":"ERR_PERMISSION_DENIED","errMsg":"Permission denied","suggestion":"Ensure the caller has the required permission (ohos.permission.POWER_MANAGER or ohos.permission.POWER_OPTIMIZATION)"}
```

## Error Codes

| Error Code | Value | Description |
|---|---|---|
| ERR_OK | 0 | Operation succeeded |
| ERR_FAILURE | 1 | General failure |
| ERR_PERMISSION_DENIED | 201 | Permission denied |
| ERR_SYSTEM_API_DENIED | 202 | System API denied |
| ERR_PARAM_INVALID | 401 | Invalid parameter |
| ERR_CONNECTION_FAIL | 4900101 | Connection to power service failed |
| ERR_FREQUENT_FUNCTION_CALL | 4900201 | Too frequent function calls |
| ERR_POWER_MODE_TRANSIT_FAILED | 4900301 | Power mode transition failed |
| ERR_SKIP_FUNCTION_CALL | 4900401 | Function call skipped |
| ERR_READ_OPERATION_FAILED | 4900501 | Read operation failed |
| ERR_USER_PARAM_INVALID | 4900400 | User parameter value invalid |
| ERR_WRITE_OPERATION_FAILED | 4900601 | Write operation failed |
