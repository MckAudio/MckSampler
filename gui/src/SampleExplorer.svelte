<script>
    import { onMount, onDestroy } from "svelte";
    import Select from "./mck/controls/Select.svelte";
    import Button from "./mck/controls/Button.svelte";
    import WaveForm from "./mck/dsp/WaveForm.svelte";

    export let data = undefined;
    export let samples = undefined;
    export let sampleInfo = undefined;

    let samplesReady = false;
    $: samplesReady = samples !== undefined && typeof samples == "object";

    let infoReady = false;
    $:infoReady = sampleInfo !== undefined && activeSample !== undefined && sampleInfo.valid;

    let packs = [];
    let activePack = undefined;
    let categories = [];
    let activeCategory = undefined;
    let categoryName = "";
    let activeSample = undefined;

    /*
    $: if (sampleInfo !== undefined && sampleInfo.valid)
    {
        infoReady = true;
        activePack = sampleInfo.packIdx;
        activeSample = sampleInfo.sampleIdx;
        activeCategory = samples[activePack].samples[activeSample].type;
    }*/

    $: if (samplesReady) {
        packs = Array.from(samples, (_pack) => _pack.name);
        if (activePack >= packs.length) {
            activePack = undefined;
        } else if (activePack === undefined && packs.length > 0) {
            activePack = 0;
        }

        if (activePack !== undefined) {
            categories = samples[activePack].categories;
            if (activeCategory >= categories.length) {
                activeCategory = undefined;
                categoryName = "";
            } else if (activeCategory === undefined && categories.length > 0) {
                activeCategory = 0;
                categoryName = categories[activeCategory];
            }
        } else {
            categories = [];
            categoryName = "";
        }
    }

    function SelectSample(_idx) {
        SendMessage({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "load",
                packIdx: activePack,
                sampleIdx: _idx
            })
        });
        activeSample = _idx;
    }

    function PlaySample(_idx) {
        _idx = _idx !== undefined ? _idx : activeSample;
        SendMessage({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "play",
                packIdx: activePack,
                sampleIdx: _idx
            })
        });
        activeSample = _idx;
    }
    
    function StopSample(_idx) {
        _idx = _idx !== undefined ? _idx : activeSample;
        SendMessage({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "stop",
                packIdx: activePack,
                sampleIdx: _idx
            })
        });
    }


    onMount(() => {
        if (SendMessage) {
            SendMessage({
                section: "samples",
                msgType: "get",
                data: "",
            });
        }
    });
</script>

<div class="main">
    <div class="overview">
        <div class="label">Pack:</div>
        <Select
            items={packs}
            value={activePack}
            Handler={(_val) => {
                activeSample = undefined;
                activePack = _val;
            }}
        />

        {#if activePack !== undefined}
            <div class="label">Category:</div>
            <Select
                items={categories}
                value={activeCategory}
                Handler={(_val) => {
                    activeSample = undefined;
                    activeCategory = _val;
                }}
            />
            <div class="label">Samples:</div>
            <div class="table">
                {#each samples[activePack].samples as sample, i}
                    {#if sample.type === activeCategory}
                        <i>{sample.index}</i>
                        <span>{sample.name}</span>
                        <Button Handler={() => PlaySample(i)} title="Preview"></Button>
                        <Button value={i === activeSample} Handler={() => SelectSample(i)} title="Select"></Button>
                    {/if}
                {/each}
            </div>
        {:else}
            <div />
            <div />
        {/if}
    </div>
    {#if infoReady}
        <div class="detail">
            <div class="label">Name:</div>
            <div class="text">{samples[activePack].samples[activeSample].name}</div>
            <div class="label">Channels:</div>
            <div class="text">{sampleInfo.numChans}</div>
            <div class="label">Length:</div>
            <div class="text">{sampleInfo.lengthMs} ms</div>
            <div class="label">SampleRate:</div>
            <div class="text">{sampleInfo.sampleRate}</div>
            <div class="wave">
                <WaveForm data={sampleInfo.waveForm}/>
            </div>
            <div class="buttons">
                <Button Handler={() => StopSample()}>Stop</Button>
                <Button Handler={() => PlaySample()}>Play</Button>
                <Button>Assign</Button>
            </div>
        </div>
    {/if}
</div>

<style>
    .main {
        overflow: hidden;
        width: 100%;
        height: 100%;
        display: grid;
        grid-template-columns: 1fr 2fr;
        grid-column-gap: 16px;
    }
    .overview {
        overflow: hidden;
        display: grid;
        grid-gap: 8px;
        grid-template-columns: auto 1fr;
        grid-template-rows: auto auto 1fr;
    }
    .table {
        overflow-y: scroll;
        display: grid;
        grid-template-columns: auto 1fr auto auto;
        grid-auto-rows: auto;
        grid-gap: 8px;
    }
    .detail {
        overflow-x: hidden;
        overflow-y: auto;
        display: grid;
        grid-gap: 8px;
        grid-template-columns: auto 1fr;
        grid-template-rows: repeat(4, auto) minmax(50px, 200px) auto 1fr;
    }
    .buttons {
        grid-column: 1/-1;
        width: 100%;
        height: 100%;
        display: grid;
        grid-gap: 8px;
        grid-auto-flow: column;
        grid-auto-columns: min-content;
    }
    .wave {
        overflow: hidden;
        grid-column: 1/-1;
    }
    span,
    i, .text, .label {
        font-family: mck-lato;
        font-size: 14px;
        line-height: 30px;
    }
    .label {
        font-style: italic;
        text-align: right;
    }
    .text {
        text-align: left;
    }
</style>
