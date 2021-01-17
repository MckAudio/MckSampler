<script>
	import { onMount, onDestroy } from "svelte";

	import Settings from "./Settings.svelte";
	import Controls from "./Controls.svelte";
	import Sequencer from "./Sequencer.svelte";
	import Pads from "./Pads.svelte";

	import { SelectedPad } from "./Stores.js";

	let data = undefined;
	let dataReady = false;
	let transport = undefined;
	let transportReady = false;
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

	function ReceiveBackendMessage(_event) {
		if (
			_event.detail.section === "data" &&
			_event.detail.msgType === "full"
		) {
			data = _event.detail.data;
			dataReady = true;
			console.log("MSG", JSON.stringify(_event.detail));
		} else if (
			_event.detail.section === "transport" &&
			_event.detail.msgType === "realtime"
		) {
			transport = _event.detail.data;
			transportReady = true;
		} else {
			console.log("MSG", JSON.stringify(_event.detail));
		}
	}

	onMount(() => {
		document.addEventListener("backendMessage", ReceiveBackendMessage);
		if (SendMessage) {
			SendMessage({
				section: "data",
				msgType: "get",
				data: ""
			});
		}

		document.addEventListener(
			"touchstart",
			(_evt) => {
				_evt.preventDefault();
			},
			{ passive: false }
		);
	});

	onDestroy(() => {
		document.removeEventListener("backendMessage", ReceiveBackendMessage);
	});
</script>

<main>
	{#if dataReady}
		<div class="settings">
			<Settings {transport} />
		</div>
		<div
			class="content"
			bind:this={content}
			bind:clientHeight={contentHeight}
			bind:clientWidth={contentWidth}
		>
			<Controls {data} />
			<Sequencer {data} {transport}/>
			<Pads {data} />
		</div>
		<div class="master" />
	{/if}
</main>

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
		padding: 16px;
		grid-gap: 16px;
		background-color: #fafafa;
		display: grid;
		grid-template-rows: 1fr auto auto;
	}
</style>
