<script lang="ts">
    import SelectorPad from "../../src/mck/controls/SelectorPad.svelte";
    import { TransportState } from "./types/Transport";
    import { SendToBackend } from "./tools/Backend.svelte";

    export let style: "dark" | "light" | "custom" = "dark";
    export let transport = new TransportState();

    let pauseIdx = 0;

    function SendTransCmd(_idx) {
        SendToBackend({
            section: "transport",
            msgType: "command",
            data: JSON.stringify({
                mode: _idx,
                tempo: transport.tempo,
            }),
        });
    }
</script>

<main class="main {style}">
    <div class="label">{transport.beat + 1} | {transport.nBeats}</div>
    {#if pauseIdx === 1}
        <SelectorPad
            selected={transport.state == 1}
            {style}
            label="Continue"
            Handler={() => {
                pauseIdx = 0;
                SendTransCmd(3);
            }}
        />
    {:else}
        <SelectorPad
            selected={transport.state == 1}
            {style}
            label="Play"
            Handler={() => {
                pauseIdx = 0;
                SendTransCmd(2);
            }}
        />
    {/if}
    <SelectorPad
        {style}
        label="Stop"
        Handler={() => {
            SendTransCmd(1);
            pauseIdx += 1;
        }}
    />
</main>

<style>
    .main {
        height: calc(100% - 16px);
        width: calc(100% - 16px);
        display: grid;
        grid-template-rows: 24px repeat(3, 48px) 1fr;
        gap: 16px;
        margin: 8px;
    }
    .main.dark {
        color: #f0f0f0;
    }
    .label {
        text-align: center;
        font-weight: bold;
    }
</style>
