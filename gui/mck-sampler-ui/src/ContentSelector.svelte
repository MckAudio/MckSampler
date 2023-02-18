<script lang="ts">
    import SelectorPad from "../../src/mck/controls/SelectorPad.svelte";
    import TwoChoicePad from "../../src/mck/controls/TwoChoicePad.svelte";

    export let style: "dark" | "light" | "custom" = "light";
    export let activeContent = -1;

    let pages = ["Controls", "Settings", "Pads", "Mixer", "Sequence"];
    $: console.log(style);
</script>

<div class="main {style}">
    <div class="label">MckAudio</div>
    {#each pages as page, i}
        <SelectorPad
            {style}
            label={page}
            selected={i === activeContent}
            Handler={() => {
                activeContent = i;
            }}
        />
    {/each}
</div>

<style>
    .main {
        height: calc(100% - 16px);
        width: calc(100% - 16px);
        display: grid;
        grid-auto-rows: 24px;
        gap: 16px;
        margin: 8px;
    }
    .label {
        font-family: 'mck-lato', 'Lato';
        font-style: italic;
        font-size: 14px;
        line-height: 24px;
        text-align: center;
    }
    .main.light .label {
        color: #555;
    }
    .main.dark .label {
        color: #ff9900;
    }
</style>
