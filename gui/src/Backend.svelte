<script context="module">
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
        SendMessage({ section: "data", msgType: "patch", data: JSON.stringify(_patches) });
    }
</script>