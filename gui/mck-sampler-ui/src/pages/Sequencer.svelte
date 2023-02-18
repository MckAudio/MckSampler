<script lang="ts">
    import { onDestroy, onMount } from "svelte/internal";
    import TogglePad from "../../../src/mck/controls/TogglePad.svelte";
    import { SamplerConfig } from "../types/Sampler";

    export let style: "dark" | "light" | "custom" = "dark";
    export let config = new SamplerConfig();

    let arr = Array.from({ length: 16 }, (_, i) => [i + 1, false]);
    let selA = -1;
    let padInterval = -1;

    onMount(() => {
        //padInterval = window.setInterval(() => {selA = (selA + 1) % 16}, 300);
    });

    onDestroy(() => {
        window.clearInterval(padInterval);
    });
</script>

<main class="main">
    {#each arr as a, i}
        <TogglePad
            {style}
            label={a[0].toString()}
            selected={i === selA}
            emphasize={i%4 === 0}
            active={a[1]}
            Handler={(v) => {
                a[1] = v;
            }}
        />
    {/each}
</main>

<style>
    .main {
        width: calc(100% - 16px);
        height: calc(100% - 16px);
        padding: 8px;

        display: grid;
        grid-template-columns: repeat(8, 1fr);
        grid-template-rows: auto auto 1fr;
        gap: 16px;
        user-select: none;
    }
</style>
