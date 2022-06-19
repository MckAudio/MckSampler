<script>
    import ValuePad from "./mck/controls/ValuePad.svelte";
    import { SelectedPad, SelectedPattern } from "./Stores.svelte";

    import TwoChoicePad from "./mck/controls/TwoChoicePad.svelte";
    import { SendToBackend } from "./Backend.svelte";

    import fastJsonPatch from "fast-json-patch";

    export let data = undefined;
    export let transport = undefined;
    export let showAll = false;

    let nStep = -1;
    let steps = [];
    let blanks = Array.from({ length: 15 }, (_v, _i) => _i);
    let nPads = 16;
    let body = undefined;

    let useTriplet = false;

    $: if (transport !== undefined) {
        let pulsesPer16th = transport.nPulses / 4.0;
        nStep = Math.floor(
            transport.beat * 4.0 +
                ((transport.pulse / pulsesPer16th) % transport.nPulses)
        );
    }

    $: if (data !== undefined && body !== undefined) {
        console.log("[DATA]", data);
        let _steps = [];
        let _pad = $SelectedPad;
        let _pattern = undefined;

        if (body !== undefined) {
            if (showAll) {
                body.style.gridTemplateRows = `auto repeat(${nPads}, 1fr) 1fr`;
            } else {
                body.style.gridTemplateRows = "auto 1fr 1fr";
            }
        }
        
        if (showAll) {
            _steps = Array.from(data.pads, (_p, _i) => {
                return Array.from(_p.patterns[0].steps, (_step, _j) => {
                    return {
                        pad: _i,
                        index: _j,
                        name: (_j + 1).toString(),
                        active: _step.active,
                        value: _step.velocity / 127.0,
                    };
                });
            });
        } else {
            if (_pad !== undefined) {
                if (data.pads[_pad].nPatterns > 0) {
                    _pattern = $SelectedPattern;
                    if (_pattern === undefined) {
                        _pattern = 0;
                    } else {
                        _pattern = Math.min(
                            _pattern,
                            data.pads[_pad].nPatterns - 1
                        );
                    }
                    _steps = Array.from(
                        data.pads[_pad].patterns[_pattern].steps,
                        (_p, _i) => {
                            return {
                                index: _i,
                                name: (_i + 1).toString(),
                                active: _p.active,
                                value: _p.velocity / 127.0,
                            };
                        }
                    );
                }
            }
        }

        SelectedPattern.set(_pattern);

        steps = _steps;
    }

    function SetStep(_idx, _active, _value) {
        let _data = fastJsonPatch.deepClone(data);
        let _obs = fastJsonPatch.observe(_data);
        let _pad = $SelectedPad;
        let _pattern = $SelectedPattern;

        _data.pads[_pad].patterns[_pattern].steps[_idx].active = _active;
        if (_active) {
            _data.pads[_pad].patterns[_pattern].steps[_idx].velocity =
                _value * 127.0;
        }
        let _patch = fastJsonPatch.generate(_obs);
        SendToBackend({
            section: "data",
            msgType: "patch",
            data: JSON.stringify(_patch),
        });
    }
</script>

<div class="main" bind:this={body}>
    {#if steps.length > 0}
        <div class="label">Step Sequencer:</div>
        <TwoChoicePad
            icons={["./content/music-note.svg", "./content/music-notes.svg"]}
            labels={["1/4", "1/8"]}
        />
        {#each blanks as blank}
            <div />
        {/each}
    {/if}
    {#if showAll}
        {#each steps as pattern}
            {#each pattern as step, i}
                <ValuePad
                    selected={i === nStep}
                    active={step.active}
                    value={step.value}
                    label={step.name}
                    Handler={(_active, _value) => SetStep(i, _active, _value)}
                />
            {/each}
        {/each}
    {:else}
        {#each steps as step, i}
            <ValuePad
                selected={i === nStep}
                active={step.active}
                value={step.value}
                label={step.name}
                Handler={(_active, _value) => SetStep(i, _active, _value)}
            />
        {/each}
    {/if}
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
        grid-template-rows: auto 1fr 1fr;
        overflow-y: auto;
        overflow-x: hidden;
        width: 100%;
        height: 100%;
    }
    .label {
        grid-column: 1/-1;
        font-family: mck-lato;
        font-size: 14px;
        font-weight: bold;
    }
</style>
