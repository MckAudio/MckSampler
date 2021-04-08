<script>
    import SliderLabel from "./mck/controls/SliderLabel.svelte";
    import Select from "./mck/controls/Select.svelte";
    import { DbToLog, LogToDb } from "./mck/utils/Tools.svelte";
    import { ChangeData } from "./Backend.svelte";
    import { SelectedPad } from "./Stores";

    export let data = undefined;

    let gainMin = -110.0;
    let gainMax = 6.0;
    let pad = undefined;
    let pads = Array.from({ length: 16 }, (_v, _i) => {
        return `Pad #${_i + 1}`;
    });

    $: if (data !== undefined) {
        if ($SelectedPad !== undefined && $SelectedPad < data.numPads) {
            pad = data.pads[$SelectedPad];
        } else {
            pad = undefined;
        }
    }

    function SetGain(_value) {
        let _gain = LogToDb(_value, gainMin, gainMax);
        ChangeData(["pads", $SelectedPad, "gain"], _gain);
    }
    function SetSample(_idx) {
        let _data = JSON.stringify({
            type: "sample",
            index: $SelectedPad,
            value: _idx,
        });
        SendMessage({ section: "pads", msgType: "change", data: _data });
    }
</script>

<main>
    <div class="header">Drum Controls:</div>
    <!--<Select items={pads} value={$SelectedPad} Handler={_idx => {SelectedPad.set(_idx);}}/>-->
    {#if pad !== undefined}
        <div class="settings">
            <div class="label">Gain:</div>
            <SliderLabel
                value={DbToLog(pad.gain, gainMin, gainMax)}
                label="{pad.gain.toFixed(1)} dB"
                Handler={SetGain}
            />
            <div class="label">Sample:</div>
            <div class="text">{pad.sampleName}</div>
        </div>
    {/if}
</main>

<style>
    main {
        width: 100%;
        height: 100%;
        display: grid;
        grid-template-rows: auto 1fr;
        grid-template-columns: repeat(4, 1fr);
        grid-row-gap: 8px;
        grid-column-gap: 16px;
    }
    .header {
        grid-column: 1/-1;
        font-family: mck-lato;
        font-size: 14px;
        font-weight: bold;
    }
    .settings {
        grid-column: 1/2;
        display: grid;
        grid-template-columns: 1fr 4fr;
        grid-auto-rows: 30px;
        grid-gap: 8px;
    }
    .label, .text {
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
