<script lang="ts">
    import type { SamplerConfig } from "../types/Sampler";

    import Dial from "../../../src/mck/controls/Dial.svelte";

    export let style: "dark" | "light" | "custom" = "dark";
    export let config: SamplerConfig = undefined;

    let controls = ["Pan", "FX 1", "FX 2", "Gain"];

    let rows = [0, 1, 2, 3];
    let pads = [];
    let numPads = 0;

    $: console.log("CONFIG", config);
    $: numPads = Math.min(config.numPads, 8);
    $: pads = Array.from({length: numPads}, (v, i) => config.pads[i]);

</script>

{#if config !== undefined}
    <div class="main">
        {#each rows as row}
            {#each pads as pad, i}
                {#if controls[row] === "Pan"}
                <Dial {style} label={controls[row]} value={(pad.pan / 200.0) + 0.5}/>
                {:else}
                <Dial {style} label={controls[row]} />
                {/if}
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
