<script>
    import TogglePad from "./mck/controls/TogglePad.svelte";

    export let data = undefined;

    let steps = Array.from({ length: 16 }, (_v, _i) => {
        return { index: _i, name: (_i + 1).toString(), active: false };
    });
</script>

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
        font-style: italic;
    }
</style>

<div class="main">
    <div class="label">Step Sequencer:</div>
    {#each steps as step, i}
        <TogglePad
            active={step.active}
            label={step.name}
            Handler={(_val) => {
                steps[i].active = _val;
            }} />
    {/each}
</div>
