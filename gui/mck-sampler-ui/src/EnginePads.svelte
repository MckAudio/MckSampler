<script lang="ts">
    import Pad from "../../src/mck/controls/Pad.svelte";
    import { SendToBackend } from "./tools/Backend.svelte";
    import { BackendMessage } from "./tools/Types";
    import { SamplerTrigger } from "./types/Sampler";

    export let style: "dark" | "light" | "custom" = "light";
    export let idx = -1;

    let pads = [0, 1, 2, 3, 4, 5, 6, 7];

    function PadHandler(i: number, value: number): void {
        let data = new SamplerTrigger();
        data.index = i;
        data.strength = Math.max(0.05, Math.round(value * 100.0) / 100.0);
        let msg = new BackendMessage();
        msg.section = "pads";
        msg.msgType = "trigger";
        msg.data = JSON.stringify(data);
        SendToBackend(msg);
    }
</script>

<div class="main">
    {#each pads as padIdx}
        <Pad {style} selected={idx === padIdx} Handler={(v) => PadHandler(padIdx, v)} />
    {/each}
</div>

<style>
    .main {
        width: calc(100% - 16px);
        height: calc(100% - 16px);
        display: grid;
        grid-template-columns: repeat(8, 1fr);
        gap: 16px;
        margin: 8px;
    }
</style>
