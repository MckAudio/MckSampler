<script>
    import TogglePad from "./mck/controls/TogglePad.svelte";
    import { SelectedPad } from "./Stores.js";

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

        if (_pad !== undefined) {
            if (data.pads[_pad].nPatterns > 0) {
                _steps = Array.from(data.pads[_pad].patterns[0].steps, (_p, _i) => {
                    return {
                        index: _i,
                        name: (_i + 1).toString(),
                        active: _p.active,
                        value: _p.velocity / 127.0
                    };
                });
            }
        }

        steps = _steps;
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
            Handler={(_val) => {
                steps[i].active = _val;
            }}
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
