<script lang="ts">
    import Pad from "../../src/mck/controls/Pad.svelte";

    let pads = [0, 1, 2, 3, 4, 5, 6, 7];
</script>

<main>
    {#each pads as padIdx}
        <Pad label="#{padIdx + 1}"/>
    {/each}
</main>

<style>
    main {
        width: calc(100% - 16px);
        height: calc(100% - 16px);
        display: grid;
        grid-template-columns: repeat(8, 1fr);
        gap: 16px;
        margin: 8px;
    }
</style>