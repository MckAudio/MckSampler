<script lang="ts">
  import { onMount } from "svelte";
  import EnginePads from "./EnginePads.svelte";
  import EngineSelector from "./EngineSelector.svelte";
  function ReceiveBackendMessage(event: CustomEvent) {
    console.log(JSON.stringify(event.detail));
  }

  onMount(() => {
    document.addEventListener("backendMessage", ReceiveBackendMessage);
  });
</script>

<main>
  <div class="side left" />
  <div class="side right" />
  <div class="header">
    <EngineSelector />
  </div>
  <div class="footer">
    <EnginePads />
  </div>
  <div class="content" />
</main>

<style>
  :root {
    padding: 0px;
    margin: 0px;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen,
      Ubuntu, Cantarell, "Open Sans", "Helvetica Neue", sans-serif;
  }

  main {
    padding: 0px;
    margin: 0px;
    background-color: #333333;
    width: 800px;
    height: 480px;
    display: grid;
    grid-template-columns: 80px 1fr 80px;
    grid-template-rows: 80px 1fr 80px;
    gap: 1px;
  }

  .side {
    background-color: darkorchid;
    grid-row: 1/-1;
  }
  .side.left {
    grid-column: 1/2;
  }
  .side.right {
    grid-column: -2/-1;
  }
  .header {
    background-color: aquamarine;
    grid-row: 1/2;
    grid-column: 2/-2;
  }
  .footer {
    background-color: aquamarine;
    grid-row: -2/-1;
    grid-column: 2/-2;
  }
  .content {
    grid-row: 2/-2;
    grid-column: 2/-2;
    background-color: darkolivegreen;
  }
</style>
