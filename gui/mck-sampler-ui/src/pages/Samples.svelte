<script lang="ts">
    import { ChangeData, SendToBackend } from "../tools/Backend.svelte";
    import { onDestroy, onMount } from "svelte/internal";
    import { SamplerConfig } from "../types/Sampler";
    import type { SamplePack } from "src/types/Samples";
    import SelectorPad from "../../../src/mck/controls/SelectorPad.svelte";

    export let style: "dark" | "light" | "custom" = "dark";
    export let config = new SamplerConfig();
    export let samples: Array<SamplePack> = [];
    export let idx = -1;

    let packFilter: Array<number> = [];
    let categories: Array<string> = [];

    function updateSamples()
    {
        packFilter.forEach(i => {
            let cats = [];
            samples[i].categories.forEach(v => {
                if (cats.includes(v) === false) {
                    cats.push(v);
                }
            });
            categories = cats;
        })
    }

    onMount(() => {
        SendToBackend({
            section: "samples",
            msgType: "get",
            data: "",
        });
    });
</script>

<div class="main">
    <div class="top">
        {#each samples as pack, i}
            <SelectorPad
                label={pack.name}
                selected={packFilter.includes(i)}
                Handler={() => {
                    if (packFilter.includes(i)) {
                        packFilter = packFilter.filter(v => v !== i);
                    } else {
                        packFilter = [...packFilter, i];
                    }
                    updateSamples();
                }}
            />
        {/each}
    </div>
    <div class="side">
        {#each categories as cat, i}
            <SelectorPad label={cat}/>
        {/each}
    </div>
    <div />
</div>

<style>
    .main {
        height: 100%;
        width: 100%;
        display: grid;
        grid-template-columns: auto 1fr;
        grid-template-rows: auto 3fr;
        overflow: hidden;
    }
    .top {
        grid-column: 1/-1;
        padding: 8px;
        display: grid;
        grid-auto-columns: 64px;
        grid-auto-flow: column;
        grid-template-rows: 48px;
        gap: 16px;
    }
    .side {
        padding: 8px;
        display: grid;
        grid-auto-rows: 48px;
        grid-template-columns: 64px;
        gap: 16px;
        overflow-y: auto;
    }
</style>
