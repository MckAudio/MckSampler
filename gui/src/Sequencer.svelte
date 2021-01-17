<script>
    import TogglePad from "./mck/controls/TogglePad.svelte";
    import { SelectedPad, SelectedPattern } from "./Stores.js";

    import * as jsonpatch from 'fast-json-patch/index.mjs';

    export let data = undefined;
    export let transport = undefined;

    let nStep = -1;
    let steps = [];

    $: if (transport !== undefined) {
        let pulsesPer16th = transport.nPulses / 4.0;
        nStep = Math.floor(
            transport.beat * 4.0 +
                ((transport.pulse / pulsesPer16th) % transport.nPulses)
        );
    }

    $: if (data !== undefined) {
        let _steps = [];
        let _pad = $SelectedPad;
        let _pattern = undefined;;

        if (_pad !== undefined) {
            if (data.pads[_pad].nPatterns > 0) {
                _pattern = $SelectedPattern;
                if (_pattern === undefined) {
                    _pattern = 0;
                } else {
                    _pattern = Math.min(_pattern, data.pads[_pad].nPatterns - 1);
                }
                _steps = Array.from(data.pads[_pad].patterns[_pattern].steps, (_p, _i) => {
                    return {
                        index: _i,
                        name: (_i + 1).toString(),
                        active: _p.active,
                        value: _p.velocity / 127.0
                    };
                });
            }
        }

        SelectedPattern.set(_pattern);

        steps = _steps;
    }

    function SetStep(_idx, _active, _value)
    {
        let _data = jsonpatch.deepClone(data);
        let _obs = jsonpatch.observe(_data);
        let _pad = $SelectedPad;
        let _pattern = $SelectedPattern;

        _data.pads[_pad].patterns[_pattern].steps[_idx].active = _active;
        if (_active) {
            _data.pads[_pad].patterns[_pattern].steps[_idx].velocity = _value * 127.0;
        }
        let _patch = jsonpatch.generate(_obs);
        SendMessage({
            section: "data",
            msgType: "patch",
            data: JSON.stringify(_patch)
        });
    }
</script>

<div class="main">
    {#if steps.length > 0}
    <div class="label">Step Sequencer:</div>
    {/if}
    {#each steps as step, i}
        <TogglePad
            selected={i === nStep}
            active={step.active}
            value={step.value}
            label={step.name}
            Handler={(_active, _value) => SetStep(i, _active, _value)}
        />
    {/each}
</div>

<style>
    * {
        user-select: none;
        -webkit-user-select: none;
    }
    .main {
        display: grid;
        grid-row-gap: 8px;
        grid-column-gap: 16px;
        grid-template-columns: repeat(16, 1fr);
        grid-template-rows: auto 1fr;
    }
    .label {
        grid-column: 1/-1;
        font-family: mck-lato;
        font-size: 14px;
        font-weight: bold;
    }
</style>
