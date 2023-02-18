<script lang="ts">
    import type { SamplerPad } from "../types/Sampler";
    import { ChangeData } from "../tools/Backend.svelte";

    import Dial from "../../../src/mck/controls/Dial.svelte";
    import type { DialSettings } from "../../../src/mck/controls/Types";
    import {
        DbToLog,
        LinToPan,
        LogToDb,
        PanToLin,
    } from "../../../src/mck/utils/Tools.svelte";

    export let style: "dark" | "light" | "custom" = "dark";
    export let idx: number = -1;
    export let data: SamplerPad = undefined;

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
        {
            name: "Pitch",
            key: "pitch",
            unit: "x",
            dec: 2,
            default: 1.0,
            formatValue: (val) => {
                if (val >= 0.5) {
                    return 1.0 + (val - 0.5) * 2.0 * 3.0;
                } else {
                    return 1.0 / (1.0 + (0.5 - val) * 2.0 * 3.0);
                }
            },
            extractValue: (val) => {
                if (val >= 1.0) {
                    return 0.5 + (val - 1.0) / (2.0 * 3.0);
                } else {
                    return 0.5 - (1.0 - val) / (2.0 * 3.0 * val);
                }
            },
        },
        {
            name: "Feedback",
            key: "feedback",
            sub: "delay",
            unit: "%",
            dec: 0,
            default: 10.0,
            formatValue: (val) => {
                return val * 100.0; 
            },
            extractValue: (val) => {
                return val / 100.0;
            }
        },
        {
            name: "Time",
            key: "timeMs",
            sub: "delay",
            unit: "ms",
            dec: 0,
            default: 100.0,
            formatValue: (val) => {
                return Math.pow(10.0, (val * 2.0 + 1.0));
            },
            extractValue: (val) => {
                return Math.log10(val / 10.0) * 0.5;
            }
        }
    ];
</script>

{#if data !== undefined && idx >= 0}
    <div class="main">
        {#each controls as ctrl}
        {#if ctrl.sub}
            <Dial
                {style}
                value={data[ctrl.sub][ctrl.key]}
                settings={ctrl}
                Handler={(v) => ChangeData(["pads", idx, ctrl.sub, ctrl.key], v)}
            />
            {:else}
            <Dial
                {style}
                value={data[ctrl.key]}
                settings={ctrl}
                Handler={(v) => ChangeData(["pads", idx, ctrl.key], v)}
            />
            {/if}
        {/each}
    </div>
{/if}

<style>
    .main {
        margin: 8px;
        display: grid;
        grid-template-columns: repeat(8, auto) 1fr;
        gap: 16px;
    }
</style>
