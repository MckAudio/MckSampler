<script>
    import Pad from "./mck/controls/Pad.svelte";
    import { SelectedPad } from "./Stores.js";

    export let data = undefined;

    let dataReady = false;
    let upperPads = Array.from({ length: 8 }, (_v, _i) => {
        return { index: _i, name: `Pad #${_i + 1}` };
    });
    let lowerPads = Array.from({ length: 8 }, (_v, _i) => {
        return { index: _i+8, name: `Pad #${_i + 9}` };
    });

    function PadHandler(_idx, _val) {
        SendMessage({
            section: "pads",
            msgType: "trigger",
            data: JSON.stringify({
                index: _idx,
                strength: _val,
            }),
        });
        SelectedPad.set(_idx);
    }
</script>

<style>
    * {
        user-select: none;
        -webkit-user-select: none;
    }
    .main {
        display: grid;
        grid-column-gap: 16px;
        grid-row-gap: 8px;
        grid-template-columns: repeat(8, 1fr);
        grid-template-rows: auto 0px repeat(2, 1fr);
    }
    .label {
        grid-column: 1/-1;
        font-family: mck-lato;
        font-size: 14px;
        font-weight: bold;
    }
    .empty {
        grid-column: 1/-1;
    }
</style>

<div class="main">
    <div class="label">Drum Trigger:</div>
    {#each upperPads as pad}
        <Pad selected={$SelectedPad === pad.index} label={pad.name} Handler={(_val) => PadHandler(pad.index, _val)} />
    {/each}
    <div class="empty"/>
    {#each lowerPads as pad}
        <Pad selected={$SelectedPad === pad.index} label={pad.name} Handler={(_val) => PadHandler(pad.index, _val)} />
    {/each}
</div>
