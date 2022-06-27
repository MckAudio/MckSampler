<script lang="ts">
    import SelectorPad from "../../src/mck/controls/SelectorPad.svelte";

    export let style: "dark" | "light" | "custom" = "light";
    export let idx = -1;

    let pads = [0, 1, 2, 3, 4, 5, 6, 7];
</script>

<div class="main">
    {#each pads as padIdx}
        <SelectorPad {style} label="#{padIdx + 1}" selected={padIdx === idx} Handler={() => {idx = padIdx;}}/>
    {/each}
</div>

<style>
    .main {
        width: calc(100% - 16px);
        height: calc(100% - 16px);
        display: grid;
        grid-template-columns: repeat(8, 1fr);
        gap: 16px;
        margin: 8px;
    }
</style>