<script context="module" lang="ts">
    export function ChangeData(path, value)
    {
        if (typeof(path) !== 'object') {
            path = [path];
        }
        let _patches = [{
            op: "replace",
            path: `/${path.join('/')}`,
            value: value
        }];
        SendToBackend({ section: "data", msgType: "patch", data: JSON.stringify(_patches) });
    }

    export function SendToBackend(msg: any) {
        if (window.hasOwnProperty('SendMessage')) {
            window["SendMessage"](msg);
        }
    }

    export function ShowMessageBox(msg: string) {
        if (window.hasOwnProperty('ShowMessageBox')) {
            window["ShowMessageBox"](msg);
        }
    }
</script>