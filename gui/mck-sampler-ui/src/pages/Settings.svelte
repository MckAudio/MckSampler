<script lang="ts">
    import Toggle from "../../../src/mck/controls/Toggle.svelte";
    import TextInput from "../../../src/mck/controls/TextInput.svelte";

    export let style: "dark" | "light" | "custom" = "dark";

    let jackTransport = false;

    let someContent = "Hey ho";
</script>

<div class="main {style}">
    <div class="label">Dark mode:</div>
    <Toggle
        {style}
        active={style === "dark"}
        Handler={(val) => {
            style = val ? "dark" : "light";
        }}
    />
    <div />
    <div class="label">Jack Transport:</div>
    <Toggle {style} bind:active={jackTransport} />
    <div/>
    <div class="label">Text Input:</div>
    <TextInput {style} content={someContent} name="text input"/>
</div>

<style>
    .main {
        width: calc(100% - 16px);
        height: calc(100% - 16px);
        padding: 8px;
        display: grid;
        grid-template-columns: 100px auto 1fr;
        grid-auto-rows: 32px;
        gap: 4px;
    }
    .label {
        font-family: mck-lato, "Lato";
        font-size: 14px;
        text-align: right;
        line-height: 32px;
        text-overflow: ellipsis;
        white-space: nowrap;
        overflow: hidden;
        font-style: italic;
    }
    .main.dark .label {
        color: #e0e0e0;
    }
    .main.light .label {
        color: #555555;
    }
</style>
