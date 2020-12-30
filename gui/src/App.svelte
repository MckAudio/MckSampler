<script>
	import { onMount, onDestroy } from "svelte";
	import Pad from "../MckSvelte/controls/Pad.svelte";

	let data = undefined;
	let dataReady = false;
	let pads = Array.from({ length: 16 }, (_v, _i) => {
		idx: _i;
	});
	let content = undefined;
	let contentHeight = 0;
	let oldch = 0;
	let contentWidth = 0;
	let oldcw = 0;

	$: if (contentHeight !== oldch || contentWidth !== oldcw) {
	}

	function ButtonHandler(_msg) {
		SendMessage({ message: _msg });
	}

	function PadHandler(_idx) {
		SendMessage({
			section: "pads",
			msgType: "trigger",
			data: JSON.stringify({
				index: _idx,
			}),
		});
	}

	function ReceiveBackendMessage(_event) {
		console.log("Backend Message", _event.detail);
	}

	onMount(() => {
		document.addEventListener("backendMessage", ReceiveBackendMessage);
		/*SendMessage({
			section: "data",
			msgType: "get",
			data: "hoi"
		});*/
		GetData().then((_data) => {
			console.log(JSON.stringify(_data));
			data = _data;
			dataReady = true;
		});
	});

	onDestroy(() => {
		document.removeEventListener("backendMessage", ReceiveBackendMessage);
	});
</script>

<style>
	main {
		width: 100%;
		height: 100%;
		display: grid;
		grid-template-columns: 140px 1fr auto;
	}
	.settings {
		grid-column: 1/2;
		overflow-y: auto;
		padding: 8px;
		background-color: #f0f0f0;
		z-index: 10;
		box-shadow: 1px 0px 4px 1px #555;
	}
	.content {
		grid-column: 2/3;
		background-color: #fafafa;
		display: grid;
		grid-template-rows: 1fr auto;
	}
	.pads {
		grid-row: 2/-1;
		display: grid;
		padding: 16px;
		grid-gap: 16px;
		grid-template-columns: repeat(8, 1fr);
		grid-template-rows: repeat(2, 1fr);
	}
</style>

<main>
	{#if dataReady}
		<div class="settings" />
		<div
			class="content"
			bind:this={content}
			bind:clientHeight={contentHeight}
			bind:clientWidth={contentWidth}>
			<div />
			<div class="pads">
				{#each pads as pad, i}
					<Pad Handler={() => PadHandler(i)} />
				{/each}
			</div>
		</div>
		<div class="master" />
	{/if}
</main>
