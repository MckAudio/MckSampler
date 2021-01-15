<script>
    import Button from "./mck/controls/Button.svelte";
    import SliderLabel from "./mck/controls/SliderLabel.svelte";

    export let transport = undefined;

    function SendTransCmd(_idx)
    {

    }
    function ChangeTempo(_bpm)
    {

    }
</script>

<div class="base">
    <h1>MCK Sampler</h1>

    <!-- TRANSPORT -->
    {#if transport}
            <div class="control">
                <i>Transport:</i>
                <div class="gritter">
                    <Button
                        disabled={transport.state == 1}
                        Handler={() => SendTransCmd(2)}
                    >Start</Button
                    >
                    <Button
                        disabled={transport.state == 1}
                        Handler={() => SendTransCmd(3)}
                    >Cont</Button
                    >
                    <Button
                        disabled={transport.state == 0}
                        Handler={() => SendTransCmd(1)}
                    >Stop</Button
                    >
                </div>
            </div>
            <div class="control">
                <i>Tempo:</i>
                <SliderLabel
                    label={transport.tempo.toFixed(2) + " bpm"}
                    value={(transport.tempo - 30.0) / 270.0}
                    Handler={(_v) => ChangeTempo(_v * 270.0 + 30)}
                />
            </div>
            <div class="control">
                <i>Position:</i>
                <span
                    >{(transport.bar + 1).toString().padStart(4, "0") +
                        " - " +
                        (transport.beat + 1).toString() +
                        " / " +
                        transport.nBeats.toString()}</span
                >
            </div>
        {/if}
</div>

<style>
    .base {
        display: grid;
        grid-auto-rows: auto;
        font-size: 14px;
        font-family: mck-lato;
        row-gap: 4px;
        grid-row-gap: 4px;
    }
    h1 {
        font-size: 18px;
    }
    .control {
        display: grid;
        grid-template-rows: auto minmax(24px, auto);
        grid-row-gap: 2px;
        row-gap: 2px;
        font-size: 14px;
    }
    .splitter {
        display: grid;
        grid-template-columns: 1fr 1fr;
        grid-column-gap: 2px;
        column-gap: 2px;
    }
    .gritter {
        display: grid;
        grid-auto-flow: column;
        grid-auto-columns: 1fr;
        column-gap: 2px;
    }
</style>
