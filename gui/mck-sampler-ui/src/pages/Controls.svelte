<script lang="ts">
import Dial from "../../../src/mck/controls/Dial.svelte";
import DialTest from "../../../src/mck/controls/DialTest.svelte";

export let style: "dark" | "light" | "custom" = "dark";

</script>

<div class="main">
    <Dial {style}/>
    <Dial {style}/>
    <Dial {style}/>
    <Dial {style}/>
</div>

<style>
    .main {
        margin: 8px;
        display: grid;
        grid-template-columns: repeat(4, auto) 1fr;
        gap: 16px;
    }
</style>