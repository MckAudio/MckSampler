<script>
	import { onMount, onDestroy } from "svelte";

	import Settings from "./Settings.svelte";
	import Controls from "./Controls.svelte";
	import Sequencer from "./Sequencer.svelte";
	import Pads from "./Pads.svelte";
	import Select from "./mck/controls/Select.svelte";
	import Button from "./mck/controls/Button.svelte";

	import { SelectedPad } from "./Stores.js";

	import * as jsonpatch from "fast-json-patch/index.mjs";
	import { applyOperation } from "fast-json-patch/index.mjs";
	import SampleExplorer from "./SampleExplorer.svelte";

	let data = undefined;
	let dataReady = false;
	let transport = undefined;
	let transportReady = false;
	let samples = undefined;
	let sampleInfo = undefined;
	let samplesReady = false;
    let pads = Array.from({length: 16}, (_v, _i) => {
        return `Pad #${_i+1}`;
    });
	let content = undefined;
	let contentHeight = 0;
	let oldch = 0;
	let contentWidth = 0;
	let oldcw = 0;

	let activeContent = 0;
	let activePad = undefined;

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
		} else if (_event.detail.section === "samples") {
			if (_event.detail.msgType === "packs") {
				samples = _event.detail.data;
				samplesReady = true;
			} else if (_event.detail.msgType === "info") {
				sampleInfo = _event.detail.data;
			}
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
				data: "",
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
			<div class="tabbar">
				<div class="tabs">
				<Button
					value={activeContent === 0}
					Handler={() => {
						activeContent = 0;
					}}>Controls</Button
				>
				<Button
					value={activeContent === 1}
					Handler={() => {
						activeContent = 1;
					}}>Sequencer</Button
				>
				<Button
					value={activeContent === 2}
					Handler={() => {
						activeContent = 2;
					}}>Samples</Button
				>
				</div>
				<div style="grid-column: -2/-1">
					<Select items={pads} value={$SelectedPad} Handler={_idx => {SelectedPad.set(_idx);}}/>
				</div>
			</div>
			<div class="spacer"/>
			{#if activeContent === 0}
				<Controls {data} />
			{:else if activeContent === 1}
				<Sequencer {data} {transport} />
			{:else if activeContent === 2}
				<SampleExplorer {data} {samples} {sampleInfo}/>
			{/if}
			<div class="spacer"/>
			<Pads bind:activePad {data} />
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
        overflow: hidden;
		grid-column: 2/3;
		padding: 16px;
		grid-gap: 8px;
		background-color: #fafafa;
		display: grid;
		grid-template-rows: auto 1px 1fr 1px max-content;
	}
	.spacer {
		width: 100%;
		height: 100%;
		background-color: #cccccc;
	}
	.tabbar {
		display: grid;
		grid-template-columns: 1fr minmax(80px, max-content);
		grid-gap: 8px;
	}
	.tabs {
		display: grid;
		grid-auto-flow: column;
		grid-auto-columns: min-content;
		grid-gap: 8px;
	}
</style>
