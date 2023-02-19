<script lang="ts">
  import Settings from "./pages/Settings.svelte";
  import { onMount, onDestroy } from "svelte";
  import ContentSelector from "./ContentSelector.svelte";
  import EnginePads from "./EnginePads.svelte";
  import EngineSelector from "./EngineSelector.svelte";
  import Controls from "./pages/Controls.svelte";
  import Mixer from "./pages/Mixer.svelte";
  import Pads from "./pages/Pads.svelte";
  import { SendToBackend } from "./tools/Backend.svelte";
  import type { BackendMessage } from "./tools/Types";
  import { TransportState } from "./types/Transport";
  import { SamplerConfig } from "./types/Sampler";
  import Sequencer from "./pages/Sequencer.svelte";
  import TransportBar from "./TransportBar.svelte";
    import Samples from "./pages/Samples.svelte";
    import type { SamplePack } from "./types/Samples";

  export let style: "dark" | "light" | "custom" = "dark";

  let activeContent = 0;
  let idx = 0;
  let config = new SamplerConfig();
  let transport = new TransportState();
  let samples: Array<SamplePack> = [];
  let currentSample = undefined;
  let mainElem: HTMLElement;

  let w = 0;
  let target = [800, 480];
  let h = 0;

  function ReceiveBackendMessage(event: CustomEvent) {
    let msg = event.detail as BackendMessage;

    if (msg.section === "data" && msg.msgType === "full") {
      config = msg.data as SamplerConfig;
    } else if (msg.section === "transport" && msg.msgType === "realtime") {
      transport = msg.data as TransportState;
    } else if (msg.section === "samples" && msg.msgType === "packs") {
      samples = msg.data as Array<SamplePack>;
    } else if (msg.section === "samples" && msg.msgType === "info") {
      currentSample = msg.data;
      console.log(msg.section, msg.msgType, msg.data);
    } else {
      console.log(msg.section, msg.msgType, msg.data);
    }

  }

  $: {
    let c = w/h;
    let tc = target[0]/target[1];
    let z = 1.0;

    if (c > tc) {
      z = h/target[1];
    } else {
      z = w/target[0];
    }
    if (mainElem) {
      mainElem.style.zoom = `${z*100.0}%`;
    }
    console.log(w, h, z);

  }

  onMount(() => {
    document.addEventListener("backendMessage", ReceiveBackendMessage);
    SendToBackend({
      section: "data",
      msgType: "get",
      data: "",
    });
    SendToBackend({
      section: "samples",
      msgType: "get",
      data: "",
    });
  });
  onDestroy(() => {
    document.removeEventListener("backendMessage", ReceiveBackendMessage);
  });
</script>

<div bind:clientHeight={h} bind:clientWidth={w} class="scaler">
  <div bind:this={mainElem} id="mainview" class="main {style}">
    <div class="side left">
      <ContentSelector bind:activeContent {style} />
    </div>
    <div class="side right">
      <TransportBar {style} {transport} />
    </div>
    <div class="header">
      {#if activeContent === 0 || activeContent === 4 || activeContent === 5}
        <EngineSelector {style} bind:idx />
      {/if}
    </div>
    <div class="footer">
      {#if activeContent !== 2}
        <EnginePads {style} {idx} />
      {/if}
    </div>
    <div class="content">
      {#if activeContent === 0}
        <Controls {style} {idx} data={config.pads[idx]} />
      {:else if activeContent === 1}
        <Settings bind:style />
      {:else if activeContent === 2}
        <Pads {style} />
      {:else if activeContent === 3}
        <Mixer {style} {config} />
      {:else if activeContent === 4}
        <Sequencer {style} {config} {transport} {idx} />
      {:else if activeContent === 5}
        <Samples {style} {config} {samples} {currentSample} {idx} />
      {/if}
    </div>
  </div>
</div>

<style>
  :root {
    padding: 0px;
    margin: 0px;
    font-family: "mck-lato", "Lato";
  }

  .scaler {
    width: 100%;
    height: 100%;
    display: grid;
    justify-items: center;
    align-items: center;
    overflow: hidden;
  }

  .main {
    padding: 1px;
    margin: 0px;
    width: 798px;
    height: 478px;
    display: grid;
    grid-template-columns: 80px 1fr 80px;
    grid-template-rows: 40px 1fr 80px;
    gap: 1px;
    overflow: hidden;
  }

  .main.dark {
    background-color: #3a3a3a;
  }

  .main.light {
    background-color: #cccccc;
  }

  .main.dark div {
    background-color: #2a2a2a;
  }

  .main.light div {
    background-color: #fafafa;
  }

  .side {
    grid-row: 1/-1;
    overflow: hidden;
  }
  .side.left {
    grid-column: 1/2;
  }
  .side.right {
    grid-column: -2/-1;
  }
  .header {
    grid-row: 1/2;
    grid-column: 2/-2;
    overflow: hidden;
  }
  .footer {
    grid-row: -2/-1;
    grid-column: 2/-2;
    overflow: hidden;
  }
  .content {
    grid-row: 2/-2;
    grid-column: 2/-2;
    overflow: hidden;
  }
</style>
