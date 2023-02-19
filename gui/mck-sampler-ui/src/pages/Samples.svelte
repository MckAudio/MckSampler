<script lang="ts">
    import { ChangeData, SendToBackend } from "../tools/Backend.svelte";
    import { onDestroy, onMount } from "svelte/internal";
    import { SamplerConfig } from "../types/Sampler";
    import type { SamplePack, SamplePackSample } from "src/types/Samples";
    import SelectorPad from "../../../src/mck/controls/SelectorPad.svelte";
    import { SampleSelector } from "../tools/Types";
    import WaveForm from "../../../src/mck/dsp/WaveForm.svelte";
    import { autoPlaySample } from "../tools/stores";

    export let style: "dark" | "light" | "custom" = "dark";
    export let config = new SamplerConfig();
    export let samples: Array<SamplePack> = [];
    export let currentSample = undefined;
    export let idx = -1;

    let packFilter: Array<number> = [];
    let catNames: Array<string> = [];
    let catFilter: Array<number> = [];
    let sampleNames: Array<SamplePackSample> = [];
    let sampleSel: Array<SampleSelector> = [];
    let activeSampleSel = new SampleSelector();

    function pathJoin(p1: string, p2: string): string {
        return p1 + p2.substring(1);
    }

    function updateCategories() {
        let cats = [];
        if (packFilter.length === 0) {
            samples.forEach((s) => {
                s.categories.forEach((v) => {
                    if (cats.includes(v) === false) {
                        cats.push(v);
                    }
                });
            });
        } else {
            packFilter = packFilter.sort((a, b) => a - b);
            packFilter.forEach((i) => {
                samples[i].categories.forEach((v) => {
                    if (cats.includes(v) === false) {
                        cats.push(v);
                    }
                });
            });
        }
        catNames = cats;
        updateSamples();
    }

    function updateSamples() {
        let samps: Array<SampleSelector> = [];

        catFilter = catFilter.sort((a, b) => a - b);
        /*if (catFilter.length === 0) {
            samples.forEach((pack, j) => {
                pack.samples.forEach((p, k) => {
                    let r = new SampleSelector();
                    r.name = p.name;
                    r.path = pathJoin(pack.name, p.path);
                    r.sampleIdx = k;
                    r.catIdx = p.type;
                    r.packIdx = j;
                    samps.push(r);
                });
            });
        } else */
        if (packFilter.length === 0) {
            catFilter.forEach((i) => {
                let catName = catNames[i];
                samples.forEach((pack, j) => {
                    let catIdx = pack.categories.findIndex((v) => v == catName);
                    if ((catIdx) => 0) {
                        pack.samples.forEach((p, k) => {
                            if (p.type === catIdx) {
                                let r = new SampleSelector();
                                r.name = p.name;
                                r.path = pathJoin(pack.name, p.path);
                                r.sampleIdx = k;
                                r.catIdx = p.type;
                                r.packIdx = j;
                                samps.push(r);
                            }
                        });
                    }
                });
            });
        } else {
            catFilter.forEach((i) => {
                let catName = catNames[i];
                packFilter.forEach((j) => {
                    let pack = samples[j];
                    let catIdx = pack.categories.findIndex((v) => v == catName);
                    if ((catIdx) => 0) {
                        pack.samples.forEach((p, k) => {
                            if (p.type === catIdx) {
                                let r = new SampleSelector();
                                r.name = p.name;
                                r.path = pathJoin(pack.name, p.path);
                                r.sampleIdx = k;
                                r.catIdx = p.type;
                                r.packIdx = j;
                                samps.push(r);
                            }
                        });
                    }
                });
            });
        }
        sampleSel = samps;
        console.log(sampleSel);

        if (sampleSel.findIndex((v) => v.path === activeSampleSel.path) < 0) {
            activeSampleSel = new SampleSelector();
        }
    }

    function SelectSample(_idx) {
        let _sample = sampleSel[_idx];
        SendToBackend({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "load",
                packIdx: _sample.packIdx,
                sampleIdx: _sample.sampleIdx,
                padIdx: idx,
            }),
        });
        activeSampleSel = sampleSel[_idx];
    }
    function PlaySample(_idx?: number) {
        let _sample = activeSampleSel;
        if (_idx !== undefined) {
            _sample = sampleSel[_idx];
        }
        SendToBackend({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "play",
                packIdx: _sample.packIdx,
                sampleIdx: _sample.sampleIdx,
                padIdx: idx,
            }),
        });
        activeSampleSel = _sample;
    }
    function StopSample(_idx?: number) {
        let _sample = activeSampleSel;
        if (_idx !== undefined) {
            _sample = sampleSel[_idx];
        }
        SendToBackend({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "stop",
                packIdx: _sample.packIdx,
                sampleIdx: _sample.sampleIdx,
                padIdx: idx,
            }),
        });
        activeSampleSel = _sample;
    }
    function AssignSample(_idx?: number) {
        let _sample = activeSampleSel;
        if (_idx !== undefined) {
            _sample = sampleSel[_idx];
        }
        SendToBackend({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "assign",
                packIdx: _sample.packIdx,
                sampleIdx: _sample.sampleIdx,
                padIdx: idx,
            }),
        });
        activeSampleSel = _sample;
    }

    /*
    function PlaySample(_idx) {
        let _sample = sampleSel[_idx];
        _idx = _idx !== undefined ? _idx : activeSample;
        SendToBackend({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "play",
                packIdx: activePack,
                sampleIdx: _idx,
                padIdx: $SelectedPad,
            }),
        });
        activeSample = _idx;
    }*/

    onMount(() => {
        SendToBackend({
            section: "samples",
            msgType: "get",
            data: "",
        });

        updateCategories();
    });
</script>

<div class="main {style}">
    <div class="title">Pack</div>
    <div class="title">Category</div>
    <div class="title">Samples</div>
    <div class="title">Controls</div>
    <div class="list packs">
        {#each samples as pack, i}
            <div
                class="item {packFilter.includes(i) ? 'active' : ''}"
                on:click={() => {
                    if (packFilter.includes(i)) {
                        packFilter = packFilter.filter((v) => v !== i);
                    } else {
                        packFilter = [...packFilter, i];
                    }
                    updateCategories();
                }}
            >
                {pack.name}
            </div>
        {/each}
    </div>
    <div class="list categories">
        {#each catNames as cat, i}
            <div
                class="item {catFilter.includes(i) ? 'active' : ''}"
                on:click={() => {
                    if (catFilter.includes(i)) {
                        catFilter = catFilter.filter((v) => v !== i);
                    } else {
                        catFilter = [...catFilter, i];
                    }
                    updateSamples();
                }}
            >
                {cat}
            </div>
        {/each}
    </div>
    <div class="list samples">
        {#each sampleSel as sample, i}
            <div
                class="item {config.pads[idx].samplePath === sample.path
                    ? 'highlight'
                    : ''} {activeSampleSel.path === sample.path
                    ? 'active'
                    : ''}"
                on:click={() => {
                    if ($autoPlaySample) {
                        PlaySample(i);
                    } else {
                        SelectSample(i);
                    }
                }}
            >
                {sample.name}
            </div>
        {/each}
    </div>
    <div class="info">
        {#if activeSampleSel.path !== ""}
            <SelectorPad label="Preview" Handler={() => PlaySample()} />
            <SelectorPad label="Stop" Handler={() => StopSample()} />
            <SelectorPad label="Assign" Handler={() => AssignSample()} />
        {/if}
    </div>
    {#if currentSample !== undefined && currentSample.relPath === activeSampleSel.path}
        <div class="wave">
            <WaveForm data={currentSample.waveForm} />
        </div>
    {/if}
</div>

<style>
    .main {
        height: 100%;
        width: 100%;
        display: grid;
        grid-template-columns: repeat(4, 1fr);
        grid-template-rows: auto 1fr auto;
        overflow: hidden;
    }
    .wave {
        height: 50px;
        width: 100%;
        grid-column: 1/-1;
        border-top: 1px solid #555;
    }
    .info {
        display: grid;
        padding: 8px;
        grid-auto-rows: 32px;
        grid-template-columns: 1fr 1fr;
        grid-gap: 8px;
    }
    .title {
        color: #f0f0f0;
        text-align: center;
        font-style: italic;
        border-bottom: 1px solid #505050;
        padding-bottom: 4px;
    }
    .list {
        padding: 8px;
        display: grid;
        grid-auto-flow: row;
        grid-auto-rows: 32px;
        gap: 8px;
        overflow-y: auto;
    }
    .item {
        color: #f0f0f0;
        background-color: #303030;
        cursor: pointer;
        text-align: center;
        line-height: 32px;
        border-radius: 5px;
        user-select: none;
        -webkit-user-select: none;
    }
    .item:nth-child(2n) {
        background-color: #353535;
    }
    .item.active {
        color: #ff9900;
        border-color: #dd7700;
    }
    .item.highlight {
        background-color: #663300;
    }
</style>
