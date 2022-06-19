<script>
    import { onMount, onDestroy } from "svelte";

    // DSP Elements
    import WaveForm from "./mck/dsp/WaveForm.svelte";

    // GUI Elements
    import Select from "./mck/controls/Select.svelte";
    import Button from "./mck/controls/Button.svelte";
    import InputText from "./mck/controls/InputText.svelte";

    // TOOLS
    import { SelectedPad } from "./Stores.svelte";
import { SendToBackend } from "./Backend.svelte";

    export let data = undefined;
    export let samples = undefined;
    export let sampleInfo = undefined;

    const editCmdTypes = ["CREATE", "DELETE", "CHANGE", "IMPORT", "EXPORT"];
    const editClassTypes = ["PACK", "CATEGORY", "SAMPLE"];
    const editEditTypes = ["NAME", "INDEX", "CATEGORY"];

    const editTemplate = {
        cmd: 0,
        classType: 0,
        editType: 0,
        numberValue: 0,
        stringValue: "",
        packIdx: 0,
        categoryIdx: 0,
        sampleIdx: 0,
    };

    let samplesReady = false;
    $: samplesReady = samples !== undefined && typeof samples == "object";

    let infoReady = false;
    $: infoReady =
        sampleInfo !== undefined &&
        activeSample !== undefined &&
        sampleInfo.valid;

    let packs = [];
    let activePack = undefined;
    let categories = [];
    let activeCategory = undefined;
    let categoryName = "";
    let activeSample = undefined;

    // Editing
    let editPack = false;
    let editCategory = false;
    let editSample = undefined;

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
        SendToBackend({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "load",
                packIdx: activePack,
                sampleIdx: _idx,
                padIdx: $SelectedPad,
            }),
        });
        activeSample = _idx;
    }

    function PlaySample(_idx) {
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
    }

    function StopSample(_idx) {
        _idx = _idx !== undefined ? _idx : activeSample;
        SendToBackend({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "stop",
                packIdx: activePack,
                sampleIdx: _idx,
                padIdx: $SelectedPad,
            }),
        });
    }

    function AssignSample(_idx) {
        _idx = _idx !== undefined ? _idx : activeSample;
        SendToBackend({
            section: "samples",
            msgType: "command",
            data: JSON.stringify({
                type: "assign",
                packIdx: activePack,
                sampleIdx: _idx,
                padIdx: $SelectedPad,
            }),
        });
    }

    function SendEditCmd(_cmd) {
        SendToBackend({
            section: "samples",
            msgType: "edit",
            data: JSON.stringify(_cmd),
        });
    }

    onMount(() => {
        if (SendToBackend) {
            SendToBackend({
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
        {#if editPack}
            <InputText
                value={packs[activePack]}
                Handler={(_v) => {
                    let _cmd = JSON.parse(JSON.stringify(editTemplate));
                    _cmd.cmd = 2; // Change
                    _cmd.classType = 0; // Pack
                    _cmd.editType = 0; // Name
                    _cmd.packIdx = activePack;
                    _cmd.stringValue = _v;
                    SendEditCmd(_cmd);
                }}
            />
            <Button
                title="+"
                Handler={() => {
                    let _cmd = JSON.parse(JSON.stringify(editTemplate));
                    _cmd.cmd = 0;
                    _cmd.classType = 0;
                    SendEditCmd(_cmd);
                }}
            />
            <Button
                title="Del"
                Handler={() => {
                    let _cmd = JSON.parse(JSON.stringify(editTemplate));
                    _cmd.cmd = 1; // Delete
                    _cmd.classType = 0; // Pack
                    _cmd.packIdx = activePack;
                    SendEditCmd(_cmd);
                }}
            />
        {:else}
            <div style="grid-column: 2/-2">
                <Select
                    items={packs}
                    value={activePack}
                    Handler={(_val) => {
                        activeSample = undefined;
                        activePack = _val;
                    }}
                />
            </div>
        {/if}
        <Button
            title="Edit"
            value={editPack}
            Handler={(_v) => {
                editPack = _v;
            }}
        />
        {#if activePack !== undefined}
            <div class="label">Category:</div>
            {#if editCategory}
                <InputText
                    value={categories[activeCategory]}
                    Handler={(_v) => {
                        let _cmd = JSON.parse(JSON.stringify(editTemplate));
                        _cmd.cmd = 2; // Change
                        _cmd.classType = 1; // Pack
                        _cmd.editType = 0; // Name
                        _cmd.packIdx = activePack;
                        _cmd.categoryIdx = activeCategory;
                        _cmd.stringValue = _v;
                        SendEditCmd(_cmd);
                    }}
                />
                <Button
                    title="Del"
                    Handler={() => {
                        let _cmd = JSON.parse(JSON.stringify(editTemplate));
                        _cmd.cmd = 1; // Delete
                        _cmd.classType = 1; // Pack
                        _cmd.packIdx = activePack;
                        _cmd.categoryIdx = activeCategory;
                        SendEditCmd(_cmd);
                    }}
                />
                <Button
                    title="+"
                    Handler={() => {
                        let _cmd = JSON.parse(JSON.stringify(editTemplate));
                        _cmd.cmd = 0;
                        _cmd.classType = 1;
                        _cmd.packIdx = activePack;
                        SendEditCmd(_cmd);
                    }}
                />
            {:else}
                <div style="grid-column: 2/-2">
                    <Select
                        items={categories}
                        value={activeCategory}
                        Handler={(_val) => {
                            activeSample = undefined;
                            activeCategory = _val;
                        }}
                    />
                </div>
            {/if}
            <Button
                title="Edit"
                value={editCategory}
                Handler={(_v) => {
                    editCategory = _v;
                }}
            />
            {#if activeCategory !== undefined}
                <div class="label">Samples:</div>
                <i style="grid-column: 2/-2"
                    >{activeSample !== undefined
                        ? samples[activePack].samples[activeSample].name
                        : ""}</i
                >
                <Button
                    title="+"
                    Handler={() => {
                        let _cmd = JSON.parse(JSON.stringify(editTemplate));
                        _cmd.cmd = 0;
                        _cmd.classType = 2;
                        _cmd.packIdx = activePack;
                        _cmd.categoryIdx = activeCategory;
                        SendEditCmd(_cmd);
                    }}
                />
                <div />
                <div class="table">
                    {#each samples[activePack].samples as sample, i}
                        {#if sample.type === activeCategory}
                            <i>{sample.index}</i>
                            <span>{sample.name}</span>
                            <Button
                                Handler={() => AssignSample(i)}
                                title="Assign"
                            />
                            <Button
                                Handler={() => PlaySample(i)}
                                title="Play"
                            />
                            <Button
                                value={i === activeSample}
                                Handler={() => SelectSample(i)}
                                title="Select"
                            />
                            {#if i === editSample}
                                <InputText
                                    value={sample.name}
                                    Handler={(_v) => {
                                        let _cmd = JSON.parse(
                                            JSON.stringify(editTemplate)
                                        );
                                        _cmd.cmd = 2; // Change
                                        _cmd.classType = 1; // Pack
                                        _cmd.editType = 0; // Name
                                        _cmd.packIdx = activePack;
                                        _cmd.sampleIdx = i;
                                        _cmd.stringValue = _v;
                                        SendEditCmd(_cmd);
                                    }}
                                />
                                <Button
                                    title="Del"
                                    Handler={() => {
                                        let _cmd = JSON.parse(
                                            JSON.stringify(editTemplate)
                                        );
                                        _cmd.cmd = 1; // Delete
                                        _cmd.classType = 1; // Pack
                                        _cmd.packIdx = activePack;
                                        _cmd.sampleIdx = i;
                                        SendEditCmd(_cmd);
                                    }}
                                />
                            {/if}
                        {/if}
                    {/each}
                </div>
            {/if}
        {:else}
            <div style="grid-column: 1/-1" />
        {/if}
    </div>
    {#if infoReady}
        <div class="detail">
            <div class="label">Name:</div>
            <div class="text">
                {samples[activePack].samples[activeSample].name}
            </div>
            <div class="label">Channels:</div>
            <div class="text">{sampleInfo.numChans}</div>
            <div class="label">Length:</div>
            <div class="text">{sampleInfo.lengthMs} ms</div>
            <div class="label">SampleRate:</div>
            <div class="text">{sampleInfo.sampleRate}</div>
            <div class="wave">
                <WaveForm data={sampleInfo.waveForm} />
            </div>
            <div class="buttons">
                <Button Handler={() => StopSample()}>Stop</Button>
                <Button Handler={() => PlaySample()}>Play</Button>
                <Button Handler={() => AssignSample()}>Assign</Button>
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
        grid-template-columns: 3fr 5fr;
        grid-column-gap: 16px;
    }
    .overview {
        overflow: hidden;
        display: grid;
        grid-gap: 8px;
        grid-template-columns: auto 1fr repeat(3, 48px);
        grid-template-rows: repeat(3, auto) 1fr;
    }
    .table {
        grid-column: 2/-1;
        overflow-y: scroll;
        display: grid;
        grid-template-columns: auto 1fr repeat(2, auto) 48px;
        grid-auto-rows: min-content;
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
    i,
    .text,
    .label {
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
