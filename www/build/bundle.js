
(function(l, r) { if (l.getElementById('livereloadscript')) return; r = l.createElement('script'); r.async = 1; r.src = '//' + (window.location.host || 'localhost').split(':')[0] + ':35729/livereload.js?snipver=1'; r.id = 'livereloadscript'; l.getElementsByTagName('head')[0].appendChild(r) })(window.document);
var app = (function () {
    'use strict';

    function noop() { }
    const identity = x => x;
    function add_location(element, file, line, column, char) {
        element.__svelte_meta = {
            loc: { file, line, column, char }
        };
    }
    function run(fn) {
        return fn();
    }
    function blank_object() {
        return Object.create(null);
    }
    function run_all(fns) {
        fns.forEach(run);
    }
    function is_function(thing) {
        return typeof thing === 'function';
    }
    function safe_not_equal(a, b) {
        return a != a ? b == b : a !== b || ((a && typeof a === 'object') || typeof a === 'function');
    }
    function is_empty(obj) {
        return Object.keys(obj).length === 0;
    }

    const is_client = typeof window !== 'undefined';
    let now = is_client
        ? () => window.performance.now()
        : () => Date.now();
    let raf = is_client ? cb => requestAnimationFrame(cb) : noop;

    const tasks = new Set();
    function run_tasks(now) {
        tasks.forEach(task => {
            if (!task.c(now)) {
                tasks.delete(task);
                task.f();
            }
        });
        if (tasks.size !== 0)
            raf(run_tasks);
    }
    /**
     * Creates a new task that runs on each raf frame
     * until it returns a falsy value or is aborted
     */
    function loop(callback) {
        let task;
        if (tasks.size === 0)
            raf(run_tasks);
        return {
            promise: new Promise(fulfill => {
                tasks.add(task = { c: callback, f: fulfill });
            }),
            abort() {
                tasks.delete(task);
            }
        };
    }

    function append(target, node) {
        target.appendChild(node);
    }
    function insert(target, node, anchor) {
        target.insertBefore(node, anchor || null);
    }
    function detach(node) {
        node.parentNode.removeChild(node);
    }
    function destroy_each(iterations, detaching) {
        for (let i = 0; i < iterations.length; i += 1) {
            if (iterations[i])
                iterations[i].d(detaching);
        }
    }
    function element(name) {
        return document.createElement(name);
    }
    function text(data) {
        return document.createTextNode(data);
    }
    function space() {
        return text(' ');
    }
    function listen(node, event, handler, options) {
        node.addEventListener(event, handler, options);
        return () => node.removeEventListener(event, handler, options);
    }
    function attr(node, attribute, value) {
        if (value == null)
            node.removeAttribute(attribute);
        else if (node.getAttribute(attribute) !== value)
            node.setAttribute(attribute, value);
    }
    function children(element) {
        return Array.from(element.childNodes);
    }
    function set_style(node, key, value, important) {
        node.style.setProperty(key, value, important ? 'important' : '');
    }
    // unfortunately this can't be a constant as that wouldn't be tree-shakeable
    // so we cache the result instead
    let crossorigin;
    function is_crossorigin() {
        if (crossorigin === undefined) {
            crossorigin = false;
            try {
                if (typeof window !== 'undefined' && window.parent) {
                    void window.parent.document;
                }
            }
            catch (error) {
                crossorigin = true;
            }
        }
        return crossorigin;
    }
    function add_resize_listener(node, fn) {
        const computed_style = getComputedStyle(node);
        const z_index = (parseInt(computed_style.zIndex) || 0) - 1;
        if (computed_style.position === 'static') {
            node.style.position = 'relative';
        }
        const iframe = element('iframe');
        iframe.setAttribute('style', 'display: block; position: absolute; top: 0; left: 0; width: 100%; height: 100%; ' +
            `overflow: hidden; border: 0; opacity: 0; pointer-events: none; z-index: ${z_index};`);
        iframe.setAttribute('aria-hidden', 'true');
        iframe.tabIndex = -1;
        const crossorigin = is_crossorigin();
        let unsubscribe;
        if (crossorigin) {
            iframe.src = "data:text/html,<script>onresize=function(){parent.postMessage(0,'*')}</script>";
            unsubscribe = listen(window, 'message', (event) => {
                if (event.source === iframe.contentWindow)
                    fn();
            });
        }
        else {
            iframe.src = 'about:blank';
            iframe.onload = () => {
                unsubscribe = listen(iframe.contentWindow, 'resize', fn);
            };
        }
        append(node, iframe);
        return () => {
            if (crossorigin) {
                unsubscribe();
            }
            else if (unsubscribe && iframe.contentWindow) {
                unsubscribe();
            }
            detach(iframe);
        };
    }
    function custom_event(type, detail) {
        const e = document.createEvent('CustomEvent');
        e.initCustomEvent(type, false, false, detail);
        return e;
    }

    const active_docs = new Set();
    let active = 0;
    // https://github.com/darkskyapp/string-hash/blob/master/index.js
    function hash(str) {
        let hash = 5381;
        let i = str.length;
        while (i--)
            hash = ((hash << 5) - hash) ^ str.charCodeAt(i);
        return hash >>> 0;
    }
    function create_rule(node, a, b, duration, delay, ease, fn, uid = 0) {
        const step = 16.666 / duration;
        let keyframes = '{\n';
        for (let p = 0; p <= 1; p += step) {
            const t = a + (b - a) * ease(p);
            keyframes += p * 100 + `%{${fn(t, 1 - t)}}\n`;
        }
        const rule = keyframes + `100% {${fn(b, 1 - b)}}\n}`;
        const name = `__svelte_${hash(rule)}_${uid}`;
        const doc = node.ownerDocument;
        active_docs.add(doc);
        const stylesheet = doc.__svelte_stylesheet || (doc.__svelte_stylesheet = doc.head.appendChild(element('style')).sheet);
        const current_rules = doc.__svelte_rules || (doc.__svelte_rules = {});
        if (!current_rules[name]) {
            current_rules[name] = true;
            stylesheet.insertRule(`@keyframes ${name} ${rule}`, stylesheet.cssRules.length);
        }
        const animation = node.style.animation || '';
        node.style.animation = `${animation ? `${animation}, ` : ''}${name} ${duration}ms linear ${delay}ms 1 both`;
        active += 1;
        return name;
    }
    function delete_rule(node, name) {
        const previous = (node.style.animation || '').split(', ');
        const next = previous.filter(name
            ? anim => anim.indexOf(name) < 0 // remove specific animation
            : anim => anim.indexOf('__svelte') === -1 // remove all Svelte animations
        );
        const deleted = previous.length - next.length;
        if (deleted) {
            node.style.animation = next.join(', ');
            active -= deleted;
            if (!active)
                clear_rules();
        }
    }
    function clear_rules() {
        raf(() => {
            if (active)
                return;
            active_docs.forEach(doc => {
                const stylesheet = doc.__svelte_stylesheet;
                let i = stylesheet.cssRules.length;
                while (i--)
                    stylesheet.deleteRule(i);
                doc.__svelte_rules = {};
            });
            active_docs.clear();
        });
    }

    let current_component;
    function set_current_component(component) {
        current_component = component;
    }
    function get_current_component() {
        if (!current_component)
            throw new Error('Function called outside component initialization');
        return current_component;
    }
    function onMount(fn) {
        get_current_component().$$.on_mount.push(fn);
    }
    function onDestroy(fn) {
        get_current_component().$$.on_destroy.push(fn);
    }

    const dirty_components = [];
    const binding_callbacks = [];
    const render_callbacks = [];
    const flush_callbacks = [];
    const resolved_promise = Promise.resolve();
    let update_scheduled = false;
    function schedule_update() {
        if (!update_scheduled) {
            update_scheduled = true;
            resolved_promise.then(flush);
        }
    }
    function add_render_callback(fn) {
        render_callbacks.push(fn);
    }
    let flushing = false;
    const seen_callbacks = new Set();
    function flush() {
        if (flushing)
            return;
        flushing = true;
        do {
            // first, call beforeUpdate functions
            // and update components
            for (let i = 0; i < dirty_components.length; i += 1) {
                const component = dirty_components[i];
                set_current_component(component);
                update(component.$$);
            }
            set_current_component(null);
            dirty_components.length = 0;
            while (binding_callbacks.length)
                binding_callbacks.pop()();
            // then, once components are updated, call
            // afterUpdate functions. This may cause
            // subsequent updates...
            for (let i = 0; i < render_callbacks.length; i += 1) {
                const callback = render_callbacks[i];
                if (!seen_callbacks.has(callback)) {
                    // ...so guard against infinite loops
                    seen_callbacks.add(callback);
                    callback();
                }
            }
            render_callbacks.length = 0;
        } while (dirty_components.length);
        while (flush_callbacks.length) {
            flush_callbacks.pop()();
        }
        update_scheduled = false;
        flushing = false;
        seen_callbacks.clear();
    }
    function update($$) {
        if ($$.fragment !== null) {
            $$.update();
            run_all($$.before_update);
            const dirty = $$.dirty;
            $$.dirty = [-1];
            $$.fragment && $$.fragment.p($$.ctx, dirty);
            $$.after_update.forEach(add_render_callback);
        }
    }

    let promise;
    function wait() {
        if (!promise) {
            promise = Promise.resolve();
            promise.then(() => {
                promise = null;
            });
        }
        return promise;
    }
    function dispatch(node, direction, kind) {
        node.dispatchEvent(custom_event(`${direction ? 'intro' : 'outro'}${kind}`));
    }
    const outroing = new Set();
    let outros;
    function group_outros() {
        outros = {
            r: 0,
            c: [],
            p: outros // parent group
        };
    }
    function check_outros() {
        if (!outros.r) {
            run_all(outros.c);
        }
        outros = outros.p;
    }
    function transition_in(block, local) {
        if (block && block.i) {
            outroing.delete(block);
            block.i(local);
        }
    }
    function transition_out(block, local, detach, callback) {
        if (block && block.o) {
            if (outroing.has(block))
                return;
            outroing.add(block);
            outros.c.push(() => {
                outroing.delete(block);
                if (callback) {
                    if (detach)
                        block.d(1);
                    callback();
                }
            });
            block.o(local);
        }
    }
    const null_transition = { duration: 0 };
    function create_out_transition(node, fn, params) {
        let config = fn(node, params);
        let running = true;
        let animation_name;
        const group = outros;
        group.r += 1;
        function go() {
            const { delay = 0, duration = 300, easing = identity, tick = noop, css } = config || null_transition;
            if (css)
                animation_name = create_rule(node, 1, 0, duration, delay, easing, css);
            const start_time = now() + delay;
            const end_time = start_time + duration;
            add_render_callback(() => dispatch(node, false, 'start'));
            loop(now => {
                if (running) {
                    if (now >= end_time) {
                        tick(0, 1);
                        dispatch(node, false, 'end');
                        if (!--group.r) {
                            // this will result in `end()` being called,
                            // so we don't need to clean up here
                            run_all(group.c);
                        }
                        return false;
                    }
                    if (now >= start_time) {
                        const t = easing((now - start_time) / duration);
                        tick(1 - t, t);
                    }
                }
                return running;
            });
        }
        if (is_function(config)) {
            wait().then(() => {
                // @ts-ignore
                config = config();
                go();
            });
        }
        else {
            go();
        }
        return {
            end(reset) {
                if (reset && config.tick) {
                    config.tick(1, 0);
                }
                if (running) {
                    if (animation_name)
                        delete_rule(node, animation_name);
                    running = false;
                }
            }
        };
    }
    function create_bidirectional_transition(node, fn, params, intro) {
        let config = fn(node, params);
        let t = intro ? 0 : 1;
        let running_program = null;
        let pending_program = null;
        let animation_name = null;
        function clear_animation() {
            if (animation_name)
                delete_rule(node, animation_name);
        }
        function init(program, duration) {
            const d = program.b - t;
            duration *= Math.abs(d);
            return {
                a: t,
                b: program.b,
                d,
                duration,
                start: program.start,
                end: program.start + duration,
                group: program.group
            };
        }
        function go(b) {
            const { delay = 0, duration = 300, easing = identity, tick = noop, css } = config || null_transition;
            const program = {
                start: now() + delay,
                b
            };
            if (!b) {
                // @ts-ignore todo: improve typings
                program.group = outros;
                outros.r += 1;
            }
            if (running_program || pending_program) {
                pending_program = program;
            }
            else {
                // if this is an intro, and there's a delay, we need to do
                // an initial tick and/or apply CSS animation immediately
                if (css) {
                    clear_animation();
                    animation_name = create_rule(node, t, b, duration, delay, easing, css);
                }
                if (b)
                    tick(0, 1);
                running_program = init(program, duration);
                add_render_callback(() => dispatch(node, b, 'start'));
                loop(now => {
                    if (pending_program && now > pending_program.start) {
                        running_program = init(pending_program, duration);
                        pending_program = null;
                        dispatch(node, running_program.b, 'start');
                        if (css) {
                            clear_animation();
                            animation_name = create_rule(node, t, running_program.b, running_program.duration, 0, easing, config.css);
                        }
                    }
                    if (running_program) {
                        if (now >= running_program.end) {
                            tick(t = running_program.b, 1 - t);
                            dispatch(node, running_program.b, 'end');
                            if (!pending_program) {
                                // we're done
                                if (running_program.b) {
                                    // intro — we can tidy up immediately
                                    clear_animation();
                                }
                                else {
                                    // outro — needs to be coordinated
                                    if (!--running_program.group.r)
                                        run_all(running_program.group.c);
                                }
                            }
                            running_program = null;
                        }
                        else if (now >= running_program.start) {
                            const p = now - running_program.start;
                            t = running_program.a + running_program.d * easing(p / running_program.duration);
                            tick(t, 1 - t);
                        }
                    }
                    return !!(running_program || pending_program);
                });
            }
        }
        return {
            run(b) {
                if (is_function(config)) {
                    wait().then(() => {
                        // @ts-ignore
                        config = config();
                        go(b);
                    });
                }
                else {
                    go(b);
                }
            },
            end() {
                clear_animation();
                running_program = pending_program = null;
            }
        };
    }

    const globals = (typeof window !== 'undefined'
        ? window
        : typeof globalThis !== 'undefined'
            ? globalThis
            : global);
    function create_component(block) {
        block && block.c();
    }
    function mount_component(component, target, anchor) {
        const { fragment, on_mount, on_destroy, after_update } = component.$$;
        fragment && fragment.m(target, anchor);
        // onMount happens before the initial afterUpdate
        add_render_callback(() => {
            const new_on_destroy = on_mount.map(run).filter(is_function);
            if (on_destroy) {
                on_destroy.push(...new_on_destroy);
            }
            else {
                // Edge case - component was destroyed immediately,
                // most likely as a result of a binding initialising
                run_all(new_on_destroy);
            }
            component.$$.on_mount = [];
        });
        after_update.forEach(add_render_callback);
    }
    function destroy_component(component, detaching) {
        const $$ = component.$$;
        if ($$.fragment !== null) {
            run_all($$.on_destroy);
            $$.fragment && $$.fragment.d(detaching);
            // TODO null out other refs, including component.$$ (but need to
            // preserve final state?)
            $$.on_destroy = $$.fragment = null;
            $$.ctx = [];
        }
    }
    function make_dirty(component, i) {
        if (component.$$.dirty[0] === -1) {
            dirty_components.push(component);
            schedule_update();
            component.$$.dirty.fill(0);
        }
        component.$$.dirty[(i / 31) | 0] |= (1 << (i % 31));
    }
    function init(component, options, instance, create_fragment, not_equal, props, dirty = [-1]) {
        const parent_component = current_component;
        set_current_component(component);
        const prop_values = options.props || {};
        const $$ = component.$$ = {
            fragment: null,
            ctx: null,
            // state
            props,
            update: noop,
            not_equal,
            bound: blank_object(),
            // lifecycle
            on_mount: [],
            on_destroy: [],
            before_update: [],
            after_update: [],
            context: new Map(parent_component ? parent_component.$$.context : []),
            // everything else
            callbacks: blank_object(),
            dirty,
            skip_bound: false
        };
        let ready = false;
        $$.ctx = instance
            ? instance(component, prop_values, (i, ret, ...rest) => {
                const value = rest.length ? rest[0] : ret;
                if ($$.ctx && not_equal($$.ctx[i], $$.ctx[i] = value)) {
                    if (!$$.skip_bound && $$.bound[i])
                        $$.bound[i](value);
                    if (ready)
                        make_dirty(component, i);
                }
                return ret;
            })
            : [];
        $$.update();
        ready = true;
        run_all($$.before_update);
        // `false` as a special case of no DOM component
        $$.fragment = create_fragment ? create_fragment($$.ctx) : false;
        if (options.target) {
            if (options.hydrate) {
                const nodes = children(options.target);
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                $$.fragment && $$.fragment.l(nodes);
                nodes.forEach(detach);
            }
            else {
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                $$.fragment && $$.fragment.c();
            }
            if (options.intro)
                transition_in(component.$$.fragment);
            mount_component(component, options.target, options.anchor);
            flush();
        }
        set_current_component(parent_component);
    }
    /**
     * Base class for Svelte components. Used when dev=false.
     */
    class SvelteComponent {
        $destroy() {
            destroy_component(this, 1);
            this.$destroy = noop;
        }
        $on(type, callback) {
            const callbacks = (this.$$.callbacks[type] || (this.$$.callbacks[type] = []));
            callbacks.push(callback);
            return () => {
                const index = callbacks.indexOf(callback);
                if (index !== -1)
                    callbacks.splice(index, 1);
            };
        }
        $set($$props) {
            if (this.$$set && !is_empty($$props)) {
                this.$$.skip_bound = true;
                this.$$set($$props);
                this.$$.skip_bound = false;
            }
        }
    }

    function dispatch_dev(type, detail) {
        document.dispatchEvent(custom_event(type, Object.assign({ version: '3.31.0' }, detail)));
    }
    function append_dev(target, node) {
        dispatch_dev('SvelteDOMInsert', { target, node });
        append(target, node);
    }
    function insert_dev(target, node, anchor) {
        dispatch_dev('SvelteDOMInsert', { target, node, anchor });
        insert(target, node, anchor);
    }
    function detach_dev(node) {
        dispatch_dev('SvelteDOMRemove', { node });
        detach(node);
    }
    function listen_dev(node, event, handler, options, has_prevent_default, has_stop_propagation) {
        const modifiers = options === true ? ['capture'] : options ? Array.from(Object.keys(options)) : [];
        if (has_prevent_default)
            modifiers.push('preventDefault');
        if (has_stop_propagation)
            modifiers.push('stopPropagation');
        dispatch_dev('SvelteDOMAddEventListener', { node, event, handler, modifiers });
        const dispose = listen(node, event, handler, options);
        return () => {
            dispatch_dev('SvelteDOMRemoveEventListener', { node, event, handler, modifiers });
            dispose();
        };
    }
    function attr_dev(node, attribute, value) {
        attr(node, attribute, value);
        if (value == null)
            dispatch_dev('SvelteDOMRemoveAttribute', { node, attribute });
        else
            dispatch_dev('SvelteDOMSetAttribute', { node, attribute, value });
    }
    function set_data_dev(text, data) {
        data = '' + data;
        if (text.wholeText === data)
            return;
        dispatch_dev('SvelteDOMSetData', { node: text, data });
        text.data = data;
    }
    function validate_each_argument(arg) {
        if (typeof arg !== 'string' && !(arg && typeof arg === 'object' && 'length' in arg)) {
            let msg = '{#each} only iterates over array-like objects.';
            if (typeof Symbol === 'function' && arg && Symbol.iterator in arg) {
                msg += ' You can use a spread to convert this iterable into an array.';
            }
            throw new Error(msg);
        }
    }
    function validate_slots(name, slot, keys) {
        for (const slot_key of Object.keys(slot)) {
            if (!~keys.indexOf(slot_key)) {
                console.warn(`<${name}> received an unexpected slot "${slot_key}".`);
            }
        }
    }
    /**
     * Base class for Svelte components with some minor dev-enhancements. Used when dev=true.
     */
    class SvelteComponentDev extends SvelteComponent {
        constructor(options) {
            if (!options || (!options.target && !options.$$inline)) {
                throw new Error("'target' is a required option");
            }
            super();
        }
        $destroy() {
            super.$destroy();
            this.$destroy = () => {
                console.warn('Component was already destroyed'); // eslint-disable-line no-console
            };
        }
        $capture_state() { }
        $inject_state() { }
    }

    function fade(node, { delay = 0, duration = 400, easing = identity }) {
        const o = +getComputedStyle(node).opacity;
        return {
            delay,
            duration,
            easing,
            css: t => `opacity: ${t * o}`
        };
    }

    /* MckSvelte/utils/Tools.svelte generated by Svelte v3.31.0 */

    function GetOffsetTop(elem) {
    	var offsetTop = 0;

    	do {
    		if (!isNaN(elem.offsetTop)) {
    			offsetTop += elem.offsetTop;
    		}
    	} while (elem = elem.offsetParent);

    	return offsetTop;
    }

    /* MckSvelte/controls/Pad.svelte generated by Svelte v3.31.0 */
    const file = "MckSvelte/controls/Pad.svelte";

    // (82:4) {#if active}
    function create_if_block_1(ctx) {
    	let div;
    	let div_outro;
    	let current;

    	const block = {
    		c: function create() {
    			div = element("div");
    			attr_dev(div, "class", "highlight svelte-19i60wp");
    			set_style(div, "background", "radial-gradient(#0099ffff, #f0f0f000 " + (25 + /*value*/ ctx[4] * 225) + "%)");
    			add_location(div, file, 82, 8, 2072);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			if (!current || dirty & /*value*/ 16) {
    				set_style(div, "background", "radial-gradient(#0099ffff, #f0f0f000 " + (25 + /*value*/ ctx[4] * 225) + "%)");
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			if (div_outro) div_outro.end(1);
    			current = true;
    		},
    		o: function outro(local) {
    			div_outro = create_out_transition(div, fade, {});
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			if (detaching && div_outro) div_outro.end();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_1.name,
    		type: "if",
    		source: "(82:4) {#if active}",
    		ctx
    	});

    	return block;
    }

    // (88:4) {#if label !== undefined}
    function create_if_block(ctx) {
    	let div;
    	let t;

    	const block = {
    		c: function create() {
    			div = element("div");
    			t = text(/*label*/ ctx[0]);
    			attr_dev(div, "class", "label svelte-19i60wp");
    			add_location(div, file, 88, 8, 2274);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			append_dev(div, t);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*label*/ 1) set_data_dev(t, /*label*/ ctx[0]);
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block.name,
    		type: "if",
    		source: "(88:4) {#if label !== undefined}",
    		ctx
    	});

    	return block;
    }

    function create_fragment(ctx) {
    	let main;
    	let t;
    	let main_resize_listener;
    	let current;
    	let mounted;
    	let dispose;
    	let if_block0 = /*active*/ ctx[3] && create_if_block_1(ctx);
    	let if_block1 = /*label*/ ctx[0] !== undefined && create_if_block(ctx);

    	const block = {
    		c: function create() {
    			main = element("main");
    			if (if_block0) if_block0.c();
    			t = space();
    			if (if_block1) if_block1.c();
    			set_style(main, "height", /*boxWidth*/ ctx[2] + "px");
    			attr_dev(main, "class", "svelte-19i60wp");
    			add_render_callback(() => /*main_elementresize_handler*/ ctx[9].call(main));
    			add_location(main, file, 75, 0, 1860);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, main, anchor);
    			if (if_block0) if_block0.m(main, null);
    			append_dev(main, t);
    			if (if_block1) if_block1.m(main, null);
    			/*main_binding*/ ctx[8](main);
    			main_resize_listener = add_resize_listener(main, /*main_elementresize_handler*/ ctx[9].bind(main));
    			current = true;

    			if (!mounted) {
    				dispose = [
    					listen_dev(main, "mousedown", /*mousedown_handler*/ ctx[10], false, false, false),
    					listen_dev(main, "touchstart", /*touchstart_handler*/ ctx[11], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*active*/ ctx[3]) {
    				if (if_block0) {
    					if_block0.p(ctx, dirty);

    					if (dirty & /*active*/ 8) {
    						transition_in(if_block0, 1);
    					}
    				} else {
    					if_block0 = create_if_block_1(ctx);
    					if_block0.c();
    					transition_in(if_block0, 1);
    					if_block0.m(main, t);
    				}
    			} else if (if_block0) {
    				group_outros();

    				transition_out(if_block0, 1, 1, () => {
    					if_block0 = null;
    				});

    				check_outros();
    			}

    			if (/*label*/ ctx[0] !== undefined) {
    				if (if_block1) {
    					if_block1.p(ctx, dirty);
    				} else {
    					if_block1 = create_if_block(ctx);
    					if_block1.c();
    					if_block1.m(main, null);
    				}
    			} else if (if_block1) {
    				if_block1.d(1);
    				if_block1 = null;
    			}

    			if (!current || dirty & /*boxWidth*/ 4) {
    				set_style(main, "height", /*boxWidth*/ ctx[2] + "px");
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(if_block0);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(if_block0);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(main);
    			if (if_block0) if_block0.d();
    			if (if_block1) if_block1.d();
    			/*main_binding*/ ctx[8](null);
    			main_resize_listener();
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("Pad", slots, []);
    	let { Handler = undefined } = $$props;
    	let { selected = false } = $$props;
    	let { label = undefined } = $$props;
    	let pad = undefined;
    	let boxWidth = 0;
    	let active = false;
    	let value = 0;

    	function ButtonClick(_evt) {
    		if (pad === undefined) {
    			return;
    		}

    		let _value = 0;

    		if (_evt.type === "mousedown") {
    			_value = 1 - _evt.offsetY / pad.clientHeight;
    		} else if (_evt.type === "touchstart") {
    			if (_evt.targetTouches.length > 0) {
    				let _top = GetOffsetTop(pad);
    				_value = 1 - (_evt.targetTouches.item(0).clientY - _top) / pad.clientHeight;
    			} else {
    				return;
    			}
    		} else {
    			return;
    		}

    		if (Handler) {
    			Handler(_value);
    		}

    		$$invalidate(4, value = _value);
    		$$invalidate(3, active = true);

    		window.setTimeout(
    			() => {
    				$$invalidate(3, active = false);
    			},
    			50
    		);
    	}

    	const writable_props = ["Handler", "selected", "label"];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console.warn(`<Pad> was created with unknown prop '${key}'`);
    	});

    	function main_binding($$value) {
    		binding_callbacks[$$value ? "unshift" : "push"](() => {
    			pad = $$value;
    			$$invalidate(1, pad);
    		});
    	}

    	function main_elementresize_handler() {
    		boxWidth = this.clientWidth;
    		$$invalidate(2, boxWidth);
    	}

    	const mousedown_handler = _evt => ButtonClick(_evt);
    	const touchstart_handler = _evt => ButtonClick(_evt);

    	$$self.$$set = $$props => {
    		if ("Handler" in $$props) $$invalidate(6, Handler = $$props.Handler);
    		if ("selected" in $$props) $$invalidate(7, selected = $$props.selected);
    		if ("label" in $$props) $$invalidate(0, label = $$props.label);
    	};

    	$$self.$capture_state = () => ({
    		fade,
    		GetOffsetTop,
    		Handler,
    		selected,
    		label,
    		pad,
    		boxWidth,
    		active,
    		value,
    		ButtonClick
    	});

    	$$self.$inject_state = $$props => {
    		if ("Handler" in $$props) $$invalidate(6, Handler = $$props.Handler);
    		if ("selected" in $$props) $$invalidate(7, selected = $$props.selected);
    		if ("label" in $$props) $$invalidate(0, label = $$props.label);
    		if ("pad" in $$props) $$invalidate(1, pad = $$props.pad);
    		if ("boxWidth" in $$props) $$invalidate(2, boxWidth = $$props.boxWidth);
    		if ("active" in $$props) $$invalidate(3, active = $$props.active);
    		if ("value" in $$props) $$invalidate(4, value = $$props.value);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		label,
    		pad,
    		boxWidth,
    		active,
    		value,
    		ButtonClick,
    		Handler,
    		selected,
    		main_binding,
    		main_elementresize_handler,
    		mousedown_handler,
    		touchstart_handler
    	];
    }

    class Pad extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance, create_fragment, safe_not_equal, { Handler: 6, selected: 7, label: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Pad",
    			options,
    			id: create_fragment.name
    		});
    	}

    	get Handler() {
    		throw new Error("<Pad>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set Handler(value) {
    		throw new Error("<Pad>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get selected() {
    		throw new Error("<Pad>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set selected(value) {
    		throw new Error("<Pad>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get label() {
    		throw new Error("<Pad>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set label(value) {
    		throw new Error("<Pad>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src/Pads.svelte generated by Svelte v3.31.0 */
    const file$1 = "src/Pads.svelte";

    function get_each_context(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[6] = list[i];
    	child_ctx[8] = i;
    	return child_ctx;
    }

    function get_each_context_1(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[6] = list[i];
    	child_ctx[8] = i;
    	return child_ctx;
    }

    // (50:4) {#each upperPads as pad, i}
    function create_each_block_1(ctx) {
    	let pad;
    	let current;

    	function func(...args) {
    		return /*func*/ ctx[3](/*i*/ ctx[8], ...args);
    	}

    	pad = new Pad({
    			props: {
    				label: /*pad*/ ctx[6].name,
    				Handler: func
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(pad.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(pad, target, anchor);
    			current = true;
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(pad.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(pad.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(pad, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block_1.name,
    		type: "each",
    		source: "(50:4) {#each upperPads as pad, i}",
    		ctx
    	});

    	return block;
    }

    // (54:4) {#each lowerPads as pad, i}
    function create_each_block(ctx) {
    	let pad;
    	let current;

    	function func_1(...args) {
    		return /*func_1*/ ctx[4](/*i*/ ctx[8], ...args);
    	}

    	pad = new Pad({
    			props: {
    				label: /*pad*/ ctx[6].name,
    				Handler: func_1
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(pad.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(pad, target, anchor);
    			current = true;
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(pad.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(pad.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(pad, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block.name,
    		type: "each",
    		source: "(54:4) {#each lowerPads as pad, i}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$1(ctx) {
    	let div2;
    	let div0;
    	let t1;
    	let t2;
    	let div1;
    	let t3;
    	let current;
    	let each_value_1 = /*upperPads*/ ctx[0];
    	validate_each_argument(each_value_1);
    	let each_blocks_1 = [];

    	for (let i = 0; i < each_value_1.length; i += 1) {
    		each_blocks_1[i] = create_each_block_1(get_each_context_1(ctx, each_value_1, i));
    	}

    	const out = i => transition_out(each_blocks_1[i], 1, 1, () => {
    		each_blocks_1[i] = null;
    	});

    	let each_value = /*lowerPads*/ ctx[1];
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block(get_each_context(ctx, each_value, i));
    	}

    	const out_1 = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	const block = {
    		c: function create() {
    			div2 = element("div");
    			div0 = element("div");
    			div0.textContent = "Drum Trigger:";
    			t1 = space();

    			for (let i = 0; i < each_blocks_1.length; i += 1) {
    				each_blocks_1[i].c();
    			}

    			t2 = space();
    			div1 = element("div");
    			t3 = space();

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			attr_dev(div0, "class", "label svelte-1t6hdpc");
    			add_location(div0, file$1, 48, 4, 1127);
    			attr_dev(div1, "class", "empty svelte-1t6hdpc");
    			add_location(div1, file$1, 52, 4, 1287);
    			attr_dev(div2, "class", "main svelte-1t6hdpc");
    			add_location(div2, file$1, 47, 0, 1104);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div2, anchor);
    			append_dev(div2, div0);
    			append_dev(div2, t1);

    			for (let i = 0; i < each_blocks_1.length; i += 1) {
    				each_blocks_1[i].m(div2, null);
    			}

    			append_dev(div2, t2);
    			append_dev(div2, div1);
    			append_dev(div2, t3);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(div2, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*upperPads, PadHandler*/ 1) {
    				each_value_1 = /*upperPads*/ ctx[0];
    				validate_each_argument(each_value_1);
    				let i;

    				for (i = 0; i < each_value_1.length; i += 1) {
    					const child_ctx = get_each_context_1(ctx, each_value_1, i);

    					if (each_blocks_1[i]) {
    						each_blocks_1[i].p(child_ctx, dirty);
    						transition_in(each_blocks_1[i], 1);
    					} else {
    						each_blocks_1[i] = create_each_block_1(child_ctx);
    						each_blocks_1[i].c();
    						transition_in(each_blocks_1[i], 1);
    						each_blocks_1[i].m(div2, t2);
    					}
    				}

    				group_outros();

    				for (i = each_value_1.length; i < each_blocks_1.length; i += 1) {
    					out(i);
    				}

    				check_outros();
    			}

    			if (dirty & /*lowerPads, PadHandler*/ 2) {
    				each_value = /*lowerPads*/ ctx[1];
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(div2, null);
    					}
    				}

    				group_outros();

    				for (i = each_value.length; i < each_blocks.length; i += 1) {
    					out_1(i);
    				}

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;

    			for (let i = 0; i < each_value_1.length; i += 1) {
    				transition_in(each_blocks_1[i]);
    			}

    			for (let i = 0; i < each_value.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			each_blocks_1 = each_blocks_1.filter(Boolean);

    			for (let i = 0; i < each_blocks_1.length; i += 1) {
    				transition_out(each_blocks_1[i]);
    			}

    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div2);
    			destroy_each(each_blocks_1, detaching);
    			destroy_each(each_blocks, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$1.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function PadHandler(_idx, _val) {
    	SendMessage({
    		section: "pads",
    		msgType: "trigger",
    		data: JSON.stringify({ index: _idx, strength: _val })
    	});
    }

    function instance$1($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("Pads", slots, []);
    	let { data = undefined } = $$props;
    	let dataReady = false;

    	let upperPads = Array.from({ length: 8 }, (_v, _i) => {
    		return { index: _i, name: `Pad #${_i + 1}` };
    	});

    	let lowerPads = Array.from({ length: 8 }, (_v, _i) => {
    		return { index: _i + 8, name: `Pad #${_i + 0}` };
    	});

    	const writable_props = ["data"];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console.warn(`<Pads> was created with unknown prop '${key}'`);
    	});

    	const func = (i, _val) => PadHandler(i, _val);
    	const func_1 = (i, _val) => PadHandler(i, _val);

    	$$self.$$set = $$props => {
    		if ("data" in $$props) $$invalidate(2, data = $$props.data);
    	};

    	$$self.$capture_state = () => ({
    		Pad,
    		data,
    		dataReady,
    		upperPads,
    		lowerPads,
    		PadHandler
    	});

    	$$self.$inject_state = $$props => {
    		if ("data" in $$props) $$invalidate(2, data = $$props.data);
    		if ("dataReady" in $$props) dataReady = $$props.dataReady;
    		if ("upperPads" in $$props) $$invalidate(0, upperPads = $$props.upperPads);
    		if ("lowerPads" in $$props) $$invalidate(1, lowerPads = $$props.lowerPads);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [upperPads, lowerPads, data, func, func_1];
    }

    class Pads extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$1, create_fragment$1, safe_not_equal, { data: 2 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Pads",
    			options,
    			id: create_fragment$1.name
    		});
    	}

    	get data() {
    		throw new Error("<Pads>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set data(value) {
    		throw new Error("<Pads>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* MckSvelte/controls/TogglePad.svelte generated by Svelte v3.31.0 */
    const file$2 = "MckSvelte/controls/TogglePad.svelte";

    // (56:4) {#if active}
    function create_if_block_1$1(ctx) {
    	let div;
    	let div_transition;
    	let current;

    	const block = {
    		c: function create() {
    			div = element("div");
    			attr_dev(div, "class", "highlight svelte-19i60wp");
    			set_style(div, "background", "radial-gradient(#0099ffff, #f0f0f000 200%)");
    			add_location(div, file$2, 56, 8, 1308);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			current = true;
    		},
    		i: function intro(local) {
    			if (current) return;

    			add_render_callback(() => {
    				if (!div_transition) div_transition = create_bidirectional_transition(div, fade, { duration: 100 }, true);
    				div_transition.run(1);
    			});

    			current = true;
    		},
    		o: function outro(local) {
    			if (!div_transition) div_transition = create_bidirectional_transition(div, fade, { duration: 100 }, false);
    			div_transition.run(0);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    			if (detaching && div_transition) div_transition.end();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block_1$1.name,
    		type: "if",
    		source: "(56:4) {#if active}",
    		ctx
    	});

    	return block;
    }

    // (62:4) {#if label}
    function create_if_block$1(ctx) {
    	let div;
    	let t;

    	const block = {
    		c: function create() {
    			div = element("div");
    			t = text(/*label*/ ctx[1]);
    			attr_dev(div, "class", "label svelte-19i60wp");
    			add_location(div, file$2, 62, 4, 1500);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div, anchor);
    			append_dev(div, t);
    		},
    		p: function update(ctx, dirty) {
    			if (dirty & /*label*/ 2) set_data_dev(t, /*label*/ ctx[1]);
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$1.name,
    		type: "if",
    		source: "(62:4) {#if label}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$2(ctx) {
    	let main;
    	let t;
    	let main_resize_listener;
    	let current;
    	let mounted;
    	let dispose;
    	let if_block0 = /*active*/ ctx[0] && create_if_block_1$1(ctx);
    	let if_block1 = /*label*/ ctx[1] && create_if_block$1(ctx);

    	const block = {
    		c: function create() {
    			main = element("main");
    			if (if_block0) if_block0.c();
    			t = space();
    			if (if_block1) if_block1.c();
    			set_style(main, "height", 2 * /*boxWidth*/ ctx[3] + "px");
    			attr_dev(main, "class", "svelte-19i60wp");
    			add_render_callback(() => /*main_elementresize_handler*/ ctx[7].call(main));
    			add_location(main, file$2, 49, 0, 1090);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, main, anchor);
    			if (if_block0) if_block0.m(main, null);
    			append_dev(main, t);
    			if (if_block1) if_block1.m(main, null);
    			/*main_binding*/ ctx[6](main);
    			main_resize_listener = add_resize_listener(main, /*main_elementresize_handler*/ ctx[7].bind(main));
    			current = true;

    			if (!mounted) {
    				dispose = [
    					listen_dev(main, "mousedown", /*mousedown_handler*/ ctx[8], false, false, false),
    					listen_dev(main, "touchstart", /*touchstart_handler*/ ctx[9], false, false, false)
    				];

    				mounted = true;
    			}
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*active*/ ctx[0]) {
    				if (if_block0) {
    					if (dirty & /*active*/ 1) {
    						transition_in(if_block0, 1);
    					}
    				} else {
    					if_block0 = create_if_block_1$1(ctx);
    					if_block0.c();
    					transition_in(if_block0, 1);
    					if_block0.m(main, t);
    				}
    			} else if (if_block0) {
    				group_outros();

    				transition_out(if_block0, 1, 1, () => {
    					if_block0 = null;
    				});

    				check_outros();
    			}

    			if (/*label*/ ctx[1]) {
    				if (if_block1) {
    					if_block1.p(ctx, dirty);
    				} else {
    					if_block1 = create_if_block$1(ctx);
    					if_block1.c();
    					if_block1.m(main, null);
    				}
    			} else if (if_block1) {
    				if_block1.d(1);
    				if_block1 = null;
    			}

    			if (!current || dirty & /*boxWidth*/ 8) {
    				set_style(main, "height", 2 * /*boxWidth*/ ctx[3] + "px");
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(if_block0);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(if_block0);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(main);
    			if (if_block0) if_block0.d();
    			if (if_block1) if_block1.d();
    			/*main_binding*/ ctx[6](null);
    			main_resize_listener();
    			mounted = false;
    			run_all(dispose);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$2.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$2($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("TogglePad", slots, []);
    	let { Handler = undefined } = $$props;
    	let { active = false } = $$props;
    	let { label = undefined } = $$props;
    	let pad = undefined;
    	let boxWidth = 0;

    	function ButtonClick(_evt) {
    		if (Handler) {
    			Handler(!active);
    		}
    	}

    	const writable_props = ["Handler", "active", "label"];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console.warn(`<TogglePad> was created with unknown prop '${key}'`);
    	});

    	function main_binding($$value) {
    		binding_callbacks[$$value ? "unshift" : "push"](() => {
    			pad = $$value;
    			$$invalidate(2, pad);
    		});
    	}

    	function main_elementresize_handler() {
    		boxWidth = this.clientWidth;
    		$$invalidate(3, boxWidth);
    	}

    	const mousedown_handler = _evt => ButtonClick();
    	const touchstart_handler = _evt => ButtonClick();

    	$$self.$$set = $$props => {
    		if ("Handler" in $$props) $$invalidate(5, Handler = $$props.Handler);
    		if ("active" in $$props) $$invalidate(0, active = $$props.active);
    		if ("label" in $$props) $$invalidate(1, label = $$props.label);
    	};

    	$$self.$capture_state = () => ({
    		fade,
    		GetOffsetTop,
    		Handler,
    		active,
    		label,
    		pad,
    		boxWidth,
    		ButtonClick
    	});

    	$$self.$inject_state = $$props => {
    		if ("Handler" in $$props) $$invalidate(5, Handler = $$props.Handler);
    		if ("active" in $$props) $$invalidate(0, active = $$props.active);
    		if ("label" in $$props) $$invalidate(1, label = $$props.label);
    		if ("pad" in $$props) $$invalidate(2, pad = $$props.pad);
    		if ("boxWidth" in $$props) $$invalidate(3, boxWidth = $$props.boxWidth);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [
    		active,
    		label,
    		pad,
    		boxWidth,
    		ButtonClick,
    		Handler,
    		main_binding,
    		main_elementresize_handler,
    		mousedown_handler,
    		touchstart_handler
    	];
    }

    class TogglePad extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$2, create_fragment$2, safe_not_equal, { Handler: 5, active: 0, label: 1 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "TogglePad",
    			options,
    			id: create_fragment$2.name
    		});
    	}

    	get Handler() {
    		throw new Error("<TogglePad>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set Handler(value) {
    		throw new Error("<TogglePad>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get active() {
    		throw new Error("<TogglePad>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set active(value) {
    		throw new Error("<TogglePad>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get label() {
    		throw new Error("<TogglePad>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set label(value) {
    		throw new Error("<TogglePad>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src/Sequencer.svelte generated by Svelte v3.31.0 */
    const file$3 = "src/Sequencer.svelte";

    function get_each_context$1(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[3] = list[i];
    	child_ctx[5] = i;
    	return child_ctx;
    }

    // (33:4) {#each steps as step, i}
    function create_each_block$1(ctx) {
    	let togglepad;
    	let current;

    	function func(...args) {
    		return /*func*/ ctx[2](/*i*/ ctx[5], ...args);
    	}

    	togglepad = new TogglePad({
    			props: {
    				active: /*step*/ ctx[3].active,
    				label: /*step*/ ctx[3].name,
    				Handler: func
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(togglepad.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(togglepad, target, anchor);
    			current = true;
    		},
    		p: function update(new_ctx, dirty) {
    			ctx = new_ctx;
    			const togglepad_changes = {};
    			if (dirty & /*steps*/ 1) togglepad_changes.active = /*step*/ ctx[3].active;
    			if (dirty & /*steps*/ 1) togglepad_changes.label = /*step*/ ctx[3].name;
    			if (dirty & /*steps*/ 1) togglepad_changes.Handler = func;
    			togglepad.$set(togglepad_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(togglepad.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(togglepad.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(togglepad, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block$1.name,
    		type: "each",
    		source: "(33:4) {#each steps as step, i}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$3(ctx) {
    	let div1;
    	let div0;
    	let t1;
    	let current;
    	let each_value = /*steps*/ ctx[0];
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block$1(get_each_context$1(ctx, each_value, i));
    	}

    	const out = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	const block = {
    		c: function create() {
    			div1 = element("div");
    			div0 = element("div");
    			div0.textContent = "Step Sequencer:";
    			t1 = space();

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			attr_dev(div0, "class", "label svelte-1a1q3uk");
    			add_location(div0, file$3, 31, 4, 692);
    			attr_dev(div1, "class", "main svelte-1a1q3uk");
    			add_location(div1, file$3, 30, 0, 669);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div1, anchor);
    			append_dev(div1, div0);
    			append_dev(div1, t1);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(div1, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*steps*/ 1) {
    				each_value = /*steps*/ ctx[0];
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context$1(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block$1(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(div1, null);
    					}
    				}

    				group_outros();

    				for (i = each_value.length; i < each_blocks.length; i += 1) {
    					out(i);
    				}

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;

    			for (let i = 0; i < each_value.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div1);
    			destroy_each(each_blocks, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$3.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$3($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("Sequencer", slots, []);
    	let { data = undefined } = $$props;

    	let steps = Array.from({ length: 16 }, (_v, _i) => {
    		return {
    			index: _i,
    			name: (_i + 1).toString(),
    			active: false
    		};
    	});

    	const writable_props = ["data"];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console.warn(`<Sequencer> was created with unknown prop '${key}'`);
    	});

    	const func = (i, _val) => {
    		$$invalidate(0, steps[i].active = _val, steps);
    	};

    	$$self.$$set = $$props => {
    		if ("data" in $$props) $$invalidate(1, data = $$props.data);
    	};

    	$$self.$capture_state = () => ({ TogglePad, data, steps });

    	$$self.$inject_state = $$props => {
    		if ("data" in $$props) $$invalidate(1, data = $$props.data);
    		if ("steps" in $$props) $$invalidate(0, steps = $$props.steps);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [steps, data, func];
    }

    class Sequencer extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$3, create_fragment$3, safe_not_equal, { data: 1 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Sequencer",
    			options,
    			id: create_fragment$3.name
    		});
    	}

    	get data() {
    		throw new Error("<Sequencer>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set data(value) {
    		throw new Error("<Sequencer>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src/App.svelte generated by Svelte v3.31.0 */

    const { console: console_1 } = globals;
    const file$4 = "src/App.svelte";

    // (74:1) {#if dataReady}
    function create_if_block$2(ctx) {
    	let div0;
    	let t0;
    	let div2;
    	let div1;
    	let t1;
    	let sequencer;
    	let t2;
    	let pads_1;
    	let div2_resize_listener;
    	let t3;
    	let div3;
    	let current;

    	sequencer = new Sequencer({
    			props: { data: /*data*/ ctx[2] },
    			$$inline: true
    		});

    	pads_1 = new Pads({
    			props: { data: /*data*/ ctx[2] },
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			div0 = element("div");
    			t0 = space();
    			div2 = element("div");
    			div1 = element("div");
    			t1 = space();
    			create_component(sequencer.$$.fragment);
    			t2 = space();
    			create_component(pads_1.$$.fragment);
    			t3 = space();
    			div3 = element("div");
    			attr_dev(div0, "class", "settings svelte-t78j8k");
    			add_location(div0, file$4, 74, 2, 1427);
    			add_location(div1, file$4, 80, 3, 1577);
    			attr_dev(div2, "class", "content svelte-t78j8k");
    			add_render_callback(() => /*div2_elementresize_handler*/ ctx[6].call(div2));
    			add_location(div2, file$4, 75, 2, 1454);
    			attr_dev(div3, "class", "master");
    			add_location(div3, file$4, 84, 2, 1637);
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div0, anchor);
    			insert_dev(target, t0, anchor);
    			insert_dev(target, div2, anchor);
    			append_dev(div2, div1);
    			append_dev(div2, t1);
    			mount_component(sequencer, div2, null);
    			append_dev(div2, t2);
    			mount_component(pads_1, div2, null);
    			/*div2_binding*/ ctx[5](div2);
    			div2_resize_listener = add_resize_listener(div2, /*div2_elementresize_handler*/ ctx[6].bind(div2));
    			insert_dev(target, t3, anchor);
    			insert_dev(target, div3, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const sequencer_changes = {};
    			if (dirty & /*data*/ 4) sequencer_changes.data = /*data*/ ctx[2];
    			sequencer.$set(sequencer_changes);
    			const pads_1_changes = {};
    			if (dirty & /*data*/ 4) pads_1_changes.data = /*data*/ ctx[2];
    			pads_1.$set(pads_1_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(sequencer.$$.fragment, local);
    			transition_in(pads_1.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(sequencer.$$.fragment, local);
    			transition_out(pads_1.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div0);
    			if (detaching) detach_dev(t0);
    			if (detaching) detach_dev(div2);
    			destroy_component(sequencer);
    			destroy_component(pads_1);
    			/*div2_binding*/ ctx[5](null);
    			div2_resize_listener();
    			if (detaching) detach_dev(t3);
    			if (detaching) detach_dev(div3);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_if_block$2.name,
    		type: "if",
    		source: "(74:1) {#if dataReady}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$4(ctx) {
    	let main;
    	let current;
    	let if_block = /*dataReady*/ ctx[3] && create_if_block$2(ctx);

    	const block = {
    		c: function create() {
    			main = element("main");
    			if (if_block) if_block.c();
    			attr_dev(main, "class", "svelte-t78j8k");
    			add_location(main, file$4, 72, 0, 1401);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, main, anchor);
    			if (if_block) if_block.m(main, null);
    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (/*dataReady*/ ctx[3]) {
    				if (if_block) {
    					if_block.p(ctx, dirty);

    					if (dirty & /*dataReady*/ 8) {
    						transition_in(if_block, 1);
    					}
    				} else {
    					if_block = create_if_block$2(ctx);
    					if_block.c();
    					transition_in(if_block, 1);
    					if_block.m(main, null);
    				}
    			} else if (if_block) {
    				group_outros();

    				transition_out(if_block, 1, 1, () => {
    					if_block = null;
    				});

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(if_block);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(if_block);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(main);
    			if (if_block) if_block.d();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$4.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function ReceiveBackendMessage(_event) {
    	console.log("Backend Message", _event.detail);
    }

    function instance$4($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots("App", slots, []);
    	let data = undefined;
    	let dataReady = true;

    	let pads = Array.from({ length: 16 }, (_v, _i) => {
    	});

    	let content = undefined;
    	let contentHeight = 0;
    	let oldch = 0;
    	let contentWidth = 0;
    	let oldcw = 0;

    	onMount(() => {
    		document.addEventListener("backendMessage", ReceiveBackendMessage);

    		if (GetData) {
    			GetData().then(_data => {
    				console.log(JSON.stringify(_data));
    				$$invalidate(2, data = _data);
    				$$invalidate(3, dataReady = true);
    			});
    		}

    		document.addEventListener(
    			"touchstart",
    			_evt => {
    				_evt.preventDefault();
    			},
    			{ passive: false }
    		);
    	});

    	onDestroy(() => {
    		document.removeEventListener("backendMessage", ReceiveBackendMessage);
    	});

    	const writable_props = [];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== "$$") console_1.warn(`<App> was created with unknown prop '${key}'`);
    	});

    	function div2_binding($$value) {
    		binding_callbacks[$$value ? "unshift" : "push"](() => {
    			content = $$value;
    			$$invalidate(4, content);
    		});
    	}

    	function div2_elementresize_handler() {
    		contentHeight = this.clientHeight;
    		contentWidth = this.clientWidth;
    		$$invalidate(0, contentHeight);
    		$$invalidate(1, contentWidth);
    	}

    	$$self.$capture_state = () => ({
    		onMount,
    		onDestroy,
    		Pads,
    		Sequencer,
    		data,
    		dataReady,
    		pads,
    		content,
    		contentHeight,
    		oldch,
    		contentWidth,
    		oldcw,
    		ReceiveBackendMessage
    	});

    	$$self.$inject_state = $$props => {
    		if ("data" in $$props) $$invalidate(2, data = $$props.data);
    		if ("dataReady" in $$props) $$invalidate(3, dataReady = $$props.dataReady);
    		if ("pads" in $$props) pads = $$props.pads;
    		if ("content" in $$props) $$invalidate(4, content = $$props.content);
    		if ("contentHeight" in $$props) $$invalidate(0, contentHeight = $$props.contentHeight);
    		if ("oldch" in $$props) $$invalidate(8, oldch = $$props.oldch);
    		if ("contentWidth" in $$props) $$invalidate(1, contentWidth = $$props.contentWidth);
    		if ("oldcw" in $$props) $$invalidate(9, oldcw = $$props.oldcw);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	$$self.$$.update = () => {
    		if ($$self.$$.dirty & /*contentHeight, contentWidth*/ 3) ;
    	};

    	return [
    		contentHeight,
    		contentWidth,
    		data,
    		dataReady,
    		content,
    		div2_binding,
    		div2_elementresize_handler
    	];
    }

    class App extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$4, create_fragment$4, safe_not_equal, {});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "App",
    			options,
    			id: create_fragment$4.name
    		});
    	}
    }

    const app = new App({
    	target: document.body,
    	props: {
    	}
    });

    return app;

}());
//# sourceMappingURL=bundle.js.map
