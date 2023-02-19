<script lang="ts">
    import Toggle from "../../../src/mck/controls/Toggle.svelte";
    import TextInput from "../../../src/mck/controls/TextInput.svelte";
    import { onMount } from "svelte";
    import { autoPlaySample } from "../tools/stores";
    import { ChangeData, SendToBackend } from "../tools/Backend.svelte";
    import { ConnectCmd, Ports } from "../types/System";
    import { SamplerConfig } from "../types/Sampler";
    import Select from "../../../src/mck/controls/Select.svelte";
    import { TransportState } from "../types/Transport";

    export let style: "dark" | "light" | "custom" = "dark";
    export let ports = new Ports();
    export let config = new SamplerConfig();
    export let transport = new TransportState();

    let someContent = "Hey ho";
    let fullscreen = false;

    let outLeftIdx: number | undefined;
    let outRightIdx: number | undefined;

    function toggleFullscreen(val) {
        let elem = document.documentElement;

        if (val) {
            if (elem.requestFullscreen) {
                elem.requestFullscreen();
            } else if (elem.webkitRequestFullscreen) {
                elem.webkitRequestFullscreen();
            }
        } else {
            if (document.exitFullscreen) {
                document.exitFullscreen();
            } else if (document.webkitExitFullscreen) {
                document.webkitExitFullscreen();
            }
        }

        fullscreen = val;
    }

    function changeOutput(right: boolean, idx: number)
    {
        let cmd = new ConnectCmd();
        cmd.channel = right ? 1 : 0;
        cmd.port = ports.outputs[idx];
        SendToBackend({
            section: "system",
            msgType: "connectOutput",
            data: JSON.stringify(cmd)
        });
    }

    $: if (config.audioLeftConnections.length > 0) {
        let idx = ports.outputs.findIndex(
            (v) => v === config.audioLeftConnections[0]
        );
        outLeftIdx = idx === -1 ? undefined : idx;
    }
    $: if (config.audioRightConnections.length > 0) {
        let idx = ports.outputs.findIndex(
            (v) => v === config.audioRightConnections[0]
        );
        outRightIdx = idx === -1 ? undefined : idx;
    }

    onMount(() => {
        fullscreen = document.webkitFullscreenElement !== null;

        SendToBackend({
            section: "system",
            msgType: "get",
            data: "",
        });
    });
</script>

<div class="main {style}">
    <div class="title">View</div>
    <div class="title">Processing</div>
    <div class="title">Misc</div>
    <div class="list">
        <div class="label">Dark mode:</div>
        <Toggle
            {style}
            active={style === "dark"}
            Handler={(val) => {
                style = val ? "dark" : "light";
            }}
        />
        <div class="label">Fullscreen:</div>
        <Toggle {style} active={fullscreen} Handler={toggleFullscreen} />
    </div>
    <div class="list">
        <div class="label">Jack Transport:</div>
        <Toggle {style} active={transport.jackTransport === 1} />
        <div class="label">Auto Play:</div>
        <Toggle
            {style}
            active={$autoPlaySample}
            Handler={(val) => {
                autoPlaySample.set(val);
            }}
        />

        <div class="label">Out Left:</div>
        <Select
            {style}
            numeric={true}
            value={outLeftIdx}
            items={ports.outputs}
            Handler={v => changeOutput(false, v)}
        />
        <div class="label">Out Right:</div>
        <Select
            {style}
            numeric={true}
            value={outRightIdx}
            items={ports.outputs}
            Handler={v => changeOutput(true, v)}
        />
    </div>
    <div class="list">
        <div class="label">Text Input:</div>
        <TextInput {style} content={someContent} name="text input" />
    </div>
</div>

<style>
    .main {
        display: grid;
        grid-template-columns: 3fr 4fr 3fr;
        grid-template-rows: auto 1fr;
    }
    .list {
        display: grid;
        grid-template-columns: auto 1fr;
        grid-auto-rows: 32px;
        gap: 4px;
        padding: 8px;
    }
    .title {
        text-align: center;
        font-style: italic;
        border-bottom: 1px solid #3a3a3a;
        padding-bottom: 4px;
    }
    .main.light .title {
        color: #555555;
        border-color: #cccccc;
    }
    .main.dark .title {
        color: #e0e0e0;
        border-color: #3a3a3a;
    }
    .label {
        font-family: mck-lato, "Lato";
        font-size: 14px;
        text-align: right;
        line-height: 32px;
        text-overflow: ellipsis;
        white-space: nowrap;
        overflow: hidden;
        font-style: italic;
    }
    .main.dark .label {
        color: #e0e0e0;
    }
    .main.light .label {
        color: #555555;
    }
</style>
