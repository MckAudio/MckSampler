<script lang="ts">
    import type { SamplerConfig } from "../types/Sampler";
    import Dial from "../../../src/mck/controls/Dial.svelte";
    import type { DialSettings } from "../../../src/mck/controls/Types";
    import {
        DbToLog,
        LinToPan,
        LogToDb,
        PanToLin,
    } from "../../../src/mck/utils/Tools.svelte";
import { ChangeData } from "../tools/Backend.svelte";

    export let style: "dark" | "light" | "custom" = "dark";
    export let config: SamplerConfig = undefined;

    let controls: Array<DialSettings> = [
        {
            name: "Pan",
            key: "pan",
            unit: "%",
            dec: 0,
            default: 0.0,
            formatValue: (val) => LinToPan(val),
            extractValue: (val) => PanToLin(val),
        },
        {
            name: "Level",
            key: "gain",
            unit: "dB",
            dec: 1,
            default: 0.0,
            formatValue: (val) => LogToDb(val, -60.0, 6.0),
            extractValue: (val) => DbToLog(val, -60.0, 6.0),
        },
    ];

    let rows = [0, 1, 2, 3];
    let pads = [];
    let numPads = 0;

    $: console.log("CONFIG", config);
    $: numPads = Math.min(config.numPads, 8);
    $: pads = Array.from({length: numPads}, (v, i) => config.pads[i]);

</script>

{#if config !== undefined}
    <div class="main">
        {#each controls as ctrl}
            {#each pads as pad, i}
                <Dial {style} settings={ctrl} value={pad[ctrl.key]} Handler={v => ChangeData(["pads", i, ctrl.key], v)} />
            {/each}
        {/each}
    </div>
{/if}

<style>
    .main {
        overflow: hidden;
        margin: 8px;
        display: grid;
        grid-template-columns: repeat(8, 1fr);
        grid-template-rows: repeat(4, 1fr);
        gap: 7px 15px;
    }
</style>
