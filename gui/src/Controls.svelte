<script>
    import SliderLabel from "./mck/controls/SliderLabel.svelte";
    import Select from "./mck/controls/Select.svelte";
    import InputNumber from "./mck/controls/InputNumber.svelte";
    import { DbToLog, FormatPan, LinToPan, LogToDb, PanToLin } from "./mck/utils/Tools.svelte";
    import { ChangeData } from "./Backend.svelte";
    import { SelectedPad } from "./Stores";
import Button from "./mck/controls/Button.svelte";

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
    function SetPan(_value) {
        let _gain = LinToPan(_value);
        ChangeData(["pads", $SelectedPad, "pan"], _gain);
    }
    function SetLength(_value) {
        ChangeData(["pads", $SelectedPad, "lengthMs"], _value);
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
            <div class="label">Sample:</div>
            <div class="text">{pad.sampleName}</div>
            <div class="label">Gain:</div>
            <SliderLabel
                value={DbToLog(pad.gain, gainMin, gainMax)}
                label="{pad.gain.toFixed(1)} dB"
                Handler={SetGain}
            />
            <div class="label">Pan:</div>
            <SliderLabel
                centered={true}
                value={PanToLin(pad.pan)}
                label="{FormatPan(pad.pan, true)}"
                Handler={SetPan}
            />
            <div class="label">Length:</div>
            <InputNumber value={pad.lengthMs} unit="ms" Handler={SetLength}/>
            <div class="label">Playback:</div>
            <Button value={pad.reverse} title="Reverse" Handler={_v => ChangeData(["pads", $SelectedPad, "reverse"], _v)}/>
        </div>
        <div class="settings">
            <div class="label">FX:</div>
            <div class="text">Delay</div>
            <div class="label">Gain:</div>
            <SliderLabel
                value={DbToLog(pad.delay.gain, gainMin, 0.0)}
                label="{pad.delay.gain.toFixed(1)} dB"
                Handler={_v => ChangeData(["pads", $SelectedPad, "delay", "gain"], LogToDb(_v, gainMin, 0.0))}
            />
            <div class="label">Feedback:</div>
            <SliderLabel
                value={pad.delay.feedback}
                label="{(pad.delay.feedback * 100.0).toFixed(1)} %"
                Handler={_v => ChangeData(["pads", $SelectedPad, "delay", "feedback"], _v)}
            />
            <div class="label">Time:</div>
            <InputNumber value={pad.delay.timeMs} unit="ms" Handler={_v => ChangeData(["pads", $SelectedPad, "delay", "timeMs"], _v)}/>
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
