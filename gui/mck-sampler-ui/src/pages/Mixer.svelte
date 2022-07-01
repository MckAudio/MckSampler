<script lang="ts">
    import Dial from "../../../src/mck/controls/Dial.svelte";

    export let style: "dark" | "light" | "custom" = "dark";

    let controls = ["Pan", "FX 1", "FX 2", "Gain"];

    let rows = [0, 1, 2, 3];
    let cols = [0, 1, 2, 3, 4, 5, 6, 7];
</script>

<div class="main">
    {#each rows as row}
        {#each cols as col}
            <Dial {style} label="{controls[row]}"/>
        {/each}
    {/each}
</div>

<style>
    .main {
        margin: 8px;
        display: grid;
        grid-template-columns: repeat(8, auto);
        gap: 8px 16px;
    }
</style>
