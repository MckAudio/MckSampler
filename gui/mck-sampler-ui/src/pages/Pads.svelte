<script lang="ts">
    import Pad from "../../../src/mck/controls/Pad.svelte";

    export let style: "dark" | "light" | "custom" = "dark";

    let padsUpper = [4, 5, 6, 7];
    let padsLower = [0, 1, 2, 3];
</script>

<div class="main">
    {#each padsUpper as pad}
        <Pad {style} />
    {/each}
    {#each padsUpper as pad}
        <div class="label">#{pad + 1}</div>
    {/each}
    {#each padsLower as pad}
        <Pad {style} />
    {/each}
    {#each padsLower as pad}
        <div class="label">#{pad + 1}</div>
    {/each}
</div>

<style>
    .main {
        margin: 8px;
        display: grid;
        grid-template-columns: repeat(4, 1fr);
        grid-template-rows: auto 28px auto 28px;
        gap: 0px 16px;
    }
    .spacer {
        grid-column: 1/-1;
        height: 100%;
        width: 100%;
        display: block;
    }
    .label {
        width: 100%;
        height: 100%;
        text-align: center;
        font-family: "mck-lato", "Lato";
        font-size: 14px;
        line-height: 28px;
        font-weight: bold;
        color: #9a9a9a;
    }
</style>
