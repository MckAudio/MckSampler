<script>
    import { onDestroy, onMount } from "svelte";

    let state = 0;
    let mouseDown = false;

    function MouseHandler(_evt) {
        console.log(_evt);

        switch (_evt.type) {
            case "mousedown":
                state = 1;
                mouseDown = true;
                break;
            case "mouseup":
                if (mouseDown) {
                    state = 2;
                    setTimeout(() => {
                        state = 0;
                    }, 550);
                }
                mouseDown = false;
                break;
            default:
                break;
        }
    }

    onMount(() => {
        window.addEventListener("mouseup", MouseHandler);
    });

    onDestroy(() => {
        window.removeEventListener("mouseup", MouseHandler);
    });
</script>

<main class="state_{state}" on:mousedown={MouseHandler} />

<style>
    main {
        width: 62px;
        height: 62px;
        border-radius: 5px;
        margin: 8px;
        cursor: pointer;
        border: 1px solid #ababab;
        background-color: #efefef;
        box-shadow: 0px 1px 3px 0px #000;
    }
    .state_0 {
        background-color: #efefef;
        box-shadow: 0px 1px 3px 0px #000;
    }
    .state_1 {
        background-color: #0099ff;
        box-shadow: 0px 1px 3px 0px #000 inset;
    }
    .state_2 {
        /*box-shadow: 0px 1px 3px 0px #000 inset;*/
        animation-duration: 500ms;
        animation-name: pressed-shadow, pressed-color;
    }

    @keyframes pressed-shadow {
        from {
            box-shadow: 0px 1px 3px 0px #000 inset;
        }
        20% {
            box-shadow: 0px 1px 2px 0px #000 inset;
        }

        40% {
            box-shadow: 0px 1px 1px 0px #000 inset;
        }

        41% {
            box-shadow: 0px 1px 1px 0px #000;
        }

        70% {
            box-shadow: 0px 1px 2px 0px #000;
        }

        to {
            box-shadow: 0px 1px 3px 0px #000;
        }
    }

    @keyframes pressed-color {
        from {
            background-color: #0099ff;
        }

        to {
            background-color: #efefef;
        }
    }
</style>
