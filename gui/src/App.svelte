<script lang="ts">
  import Pad from './mck/controls/Pad.svelte'
  let leftCount = 0;
  let rightCount = 0;
</script>


<main>
  <Pad Handler={(v) => {leftCount += Math.round(v*100.0);}}/>
  <Pad Handler={(v) => {rightCount++;}}/>
  <div class="label">{leftCount}</div>
  <div class="label">{rightCount}</div>
</main>

<style>
  main {
    height: calc(100% - 64px);
    width: calc(100% - 64px);
    background-color: orange;
    display: grid;
    padding: 32px;
    row-gap: 16px;
    column-gap: 24px;
    grid-template-columns: 1fr 1fr;
    grid-template-rows: 1fr auto;
  }
  .button {
    background-color: aquamarine;
    border-radius: 5px;
  }
  .label {
    color: darkcyan;
    font-size: 24px;
    text-align: center;
  }
</style>