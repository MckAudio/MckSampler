<script>
    export let active = false;
</script>

<main class="{active ? 'active' : ''}" on:click={() => {active = !active;}}>

</main>

<style>
    main {
        width: 64px;
        height: 64px;
        background-color: #efefef;
        border-radius: 5px;
        margin: 8px;
        box-shadow: 0px 1px 3px 0px #000;
        cursor: pointer;
    }
    .active {
        background-color: #9ff;
        box-shadow: 0px 1px 3px 0px #000 inset;
    }
</style>