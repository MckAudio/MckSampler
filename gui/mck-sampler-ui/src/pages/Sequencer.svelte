<script lang="ts">
    import { ChangeData } from "../tools/Backend.svelte";
    import { onDestroy, onMount } from "svelte/internal";
    import TogglePad from "../../../src/mck/controls/TogglePad.svelte";
    import { SamplerConfig } from "../types/Sampler";
    import { TransportState } from "..//types/Transport";

    export let style: "dark" | "light" | "custom" = "dark";
    export let config = new SamplerConfig();
    export let transport = new TransportState();
    export let idx = -1;

    let arr = Array.from({ length: 16 }, (_, i) => [i + 1, false]);
    let selA = -1;
    let padInterval = -1;

    let stepIdx = -1;

    $: console.log(transport, stepIdx);
    $: if (transport !== undefined) {
        let pulsesPer16th = transport.nPulses / 4.0;
        stepIdx = Math.floor(
            transport.beat * 4.0 +
                ((transport.pulse / pulsesPer16th) % transport.nPulses)
        );
    }
    onMount(() => {
        //padInterval = window.setInterval(() => {selA = (selA + 1) % 16}, 300);
    });

    onDestroy(() => {
        window.clearInterval(padInterval);
    });
</script>

<main class="main">
    <div class="blank"/>
    {#if idx >= 0 && idx < config.pads.length}
        {#each config.pads[idx].patterns[0].steps as step, i}
            <TogglePad
                {style}
                label={i+1}
                selected={i === stepIdx}
                emphasize={i % 4 === 0}
                active={step.active}
                Handler={(v) => {
                    ChangeData(["pads", idx, "patterns", 0, "steps", i, "active"], v);
                }}
            />
        {/each}
    {/if}
</main>

<style>
    .main {
        width: calc(100% - 16px);
        height: calc(100% - 16px);
        padding: 8px;

        display: grid;
        grid-template-columns: repeat(8, 1fr);
        grid-template-rows: 1fr 2fr 2fr 1fr;
        gap: 16px;
        user-select: none;
    }
    .blank {
        grid-column: 1/-1;
    }
</style>
