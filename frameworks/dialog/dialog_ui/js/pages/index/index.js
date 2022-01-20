import router from '@ohos.router'

export default {
    data: {
        message: router.getParams().message,
        shutdownButton: router.getParams().shutdownButton,
        rebootButton: router.getParams().rebootButton,
        cancelButton: router.getParams().cancelButton,
    },
    onInit() {
        console.info('getParams: ' + router.getParams());
    },
    onShutdown() {
        console.info('click shutdown');
        callNativeHandler("EVENT_SHUTDOWN", "shutdown");
    },
    onReboot() {
        console.info('click reboot');
        callNativeHandler("EVENT_REBOOT", "reboot");
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler("EVENT_CANCEL", "cancel");
    }
}
