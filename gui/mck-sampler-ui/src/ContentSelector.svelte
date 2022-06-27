<script lang="ts">
    import TwoChoicePad from "../../src/mck/controls/TwoChoicePad.svelte";

    export let style: "dark" | "light" | "custom" = "light";

    $: console.log(style);
</script>

<div class="main">
    <TwoChoicePad labels={["light", "dark"]} active={style=="dark"} Handler={(v) => {console.log(v); style = v ? "dark" : "light";}}/>
</div>

<style>
    .main {
        height: calc(100% - 16px);
        width: calc(100% - 16px);
        display: grid;
        grid-template-rows: auto 1fr;
        margin: 8px;
    }
</style>
