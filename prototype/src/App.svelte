<script>
  import Button from "./Button.svelte";
</script>

<main>
  <div />
  <div class="content">
    <div class="top">
      <Button />
      <Button />
      <Button />
      <Button />
      <Button />
      <Button />
      <Button />
      <Button />
    </div>
    <div class="side left">
      <Button />
      <Button />
      <Button />
      <Button />
    </div>
    <div class="main" />
    <div class="side right">
      <Button />
      <Button />
      <Button />
      <Button />
    </div>
    <div class="bottom">
      <Button />
      <Button />
      <Button />
      <Button />
      <Button />
      <Button />
      <Button />
      <Button />
    </div>
  </div>
</main>

<style>
  main {
    margin: 0px;
    padding: 0px;
    min-width: 800px;
    min-height: 480px;
    height: 100%;
    width: 100%;
    display: grid;
    grid-template-columns: 1fr 800px 1fr;
    grid-template-rows: 1fr 480px 1fr;
  }
  .content {
    grid-column: 2/3;
    grid-row: 2/3;
    background-color: #eee;
    display: grid;
    grid-template-columns: 80px 1fr 80px;
    grid-template-rows: 80px 1fr 80px;
  }
  .top,
  .main,
  .bottom {
    grid-column: 2/-2;
  }
  .top, .bottom {
    display: grid;
    grid-template-columns: repeat(8, 80px);
    grid-template-rows: 1fr;
  }
  .top {
    grid-row: 1/2;
    background-color: #ffffaa;
  }
  .main {
    grid-row: 2/3;
  }
  .bottom {
    grid-row: 3/4;
    background-color: #aaffff;
  }
  .side {
    grid-row: 2/-2;
    display: grid;
    grid-template-rows: repeat(4, 80px);
    grid-template-columns: 1fr;
  }
  .side.left {
    grid-column: 1/2;
    background-color: #ffaaff;
  }
  .side.right {
    grid-column: -2/-1;
    background-color: #aaaaff;
  }
</style>
