<script lang="ts">
  import Settings from "./pages/Settings.svelte";
  import { onMount } from "svelte";
  import ContentSelector from "./ContentSelector.svelte";
  import EnginePads from "./EnginePads.svelte";
  import EngineSelector from "./EngineSelector.svelte";
  import Controls from "./pages/Controls.svelte";
  import Mixer from "./pages/Mixer.svelte";
  import Pads from "./pages/Pads.svelte";

  export let style: "dark" | "light" | "custom" = "dark";

  let activeContent = 0;

  let idx = -1;

  function ReceiveBackendMessage(event: CustomEvent) {
    //console.log(JSON.stringify(event.detail));
  }

  onMount(() => {
    document.addEventListener("backendMessage", ReceiveBackendMessage);
  });
</script>

<div class="main {style}">
  <div class="side left">
    <ContentSelector bind:activeContent {style} />
  </div>
  <div class="side right" />
  <div class="header">
    {#if activeContent === 0}
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
      <Controls {style} />
    {:else if activeContent === 1}
      <Settings bind:style />
    {:else if activeContent === 2}
      <Pads {style} />
    {:else if activeContent === 3}
      <Mixer {style} />
    {/if}
  </div>
</div>

<style>
  :root {
    padding: 0px;
    margin: 0px;
    font-family: "mck-lato", "Lato";
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
  }
  .footer {
    grid-row: -2/-1;
    grid-column: 2/-2;
  }
  .content {
    grid-row: 2/-2;
    grid-column: 2/-2;
  }
</style>
