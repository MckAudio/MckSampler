<script>
    import SliderLabel from "./mck/controls/SliderLabel.svelte";
    import Select from "./mck/controls/Select.svelte";
    import InputNumber from "./mck/controls/InputNumber.svelte";
    import {
        DbToLog,
        FormatPan,
        LinToPan,
        LogToDb,
        PanToLin,
    } from "./mck/utils/Tools.svelte";
    import { ChangeData, SendToBackend } from "./Backend.svelte";
    import { SelectedPad } from "./Stores.svelte";
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
        SendToBackend({ section: "pads", msgType: "change", data: _data });
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
                label={FormatPan(pad.pan, true)}
                Handler={SetPan}
            />
            <div class="label">Length:</div>
            <SliderLabel value={pad.lengthMs / pad.maxLengthMs} label="{pad.lengthMs} ms" Handler={_v => ChangeData(["pads", $SelectedPad, "lengthMs"], _v * pad.maxLengthMs)} />
            <div class="label">Playback:</div>
            <Button
                value={pad.reverse}
                title="Reverse"
                Handler={(_v) =>
                    ChangeData(["pads", $SelectedPad, "reverse"], _v)}
            />
        </div>
        <div class="settings">
            <div class="label">FX:</div>
            <div class="content">
                <div class="text">Delay</div>
                <Button
                    value={pad.delay.active}
                    title={pad.delay.active ? "On" : "Off"}
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "delay", "active"],
                            _v
                        )}
                />
            </div>
            {#if pad.delay.active}
                <div class="label">Gain:</div>
                <SliderLabel
                    value={DbToLog(pad.delay.gain, gainMin, 0.0)}
                    label="{pad.delay.gain.toFixed(1)} dB"
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "delay", "gain"],
                            LogToDb(_v, gainMin, 0.0)
                        )}
                />
                <div class="label">Feedback:</div>
                <SliderLabel
                    value={pad.delay.feedback}
                    label="{(pad.delay.feedback * 100.0).toFixed(1)} %"
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "delay", "feedback"],
                            _v
                        )}
                />
                <div class="label">Time:</div>
                <SliderLabel
                    value={pad.delay.timeMs / 1000.0}
                    label="{pad.delay.timeMs.toFixed(0)} ms"
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "delay", "timeMs"],
                            _v * 1000.0
                        )}
                />
                <div class="label">Type:</div>
                <Select
                    items={["Digital", "Analogue"]}
                    value={pad.delay.type}
                    Handler={(_v) =>
                        ChangeData(["pads", $SelectedPad, "delay", "type"], _v)}
                />
            {/if}
        </div>
        <div class="settings">
            <div class="label">FX:</div>
            <div class="content">
                <div class="text">Compressor</div>
                <Button
                    value={pad.comp.active}
                    title={pad.comp.active ? "On" : "Off"}
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "comp", "active"],
                            _v
                        )}
                />
            </div>
            {#if pad.comp.active}
                <div class="label">Threshold:</div>
                <SliderLabel
                    value={DbToLog(pad.comp.threshold, -60.0, 0.0)}
                    label="{pad.comp.threshold.toFixed(1)} dB"
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "comp", "threshold"],
                            LogToDb(_v, -60.0, 0.0)
                        )}
                />
                <div class="label">Ratio:</div>
                <SliderLabel
                    value={(pad.comp.ratio - 1.0) / 9.0}
                    label="1 : {pad.comp.ratio.toFixed(1)}"
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "comp", "ratio"],
                            _v * 9.0 + 1.0
                        )}
                />
                <div class="label">Makeup:</div>
                <SliderLabel
                    value={pad.comp.makeup / 20.0}
                    label="{pad.comp.makeup.toFixed(1)} dB"
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "comp", "makeup"],
                            _v * 20.0
                        )}
                />
                <!--
                <div class="label">Attack:</div>
                <SliderLabel
                    value={pad.comp.attackMs / 500.0}
                    label="{pad.comp.attackMs.toFixed(0)} ms"
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "comp", "attackMs"],
                            _v * 500.0
                        )}
                />
                <div class="label">Release:</div>
                <SliderLabel
                    value={pad.comp.releaseMs / 1000.0}
                    label="{pad.comp.releaseMs.toFixed(0)} ms"
                    Handler={(_v) =>
                        ChangeData(
                            ["pads", $SelectedPad, "comp", "releaseMs"],
                            _v * 1000.0
                        )}
                />
                    -->
            {/if}
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
    .content {
        display: grid;
        grid-template-columns: 3fr 1fr;
        grid-gap: 8px;
    }
    .label,
    .text {
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
