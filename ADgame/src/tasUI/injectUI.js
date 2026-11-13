// create a shadow DOM to isolate CSS styles from global pollution

const host = document.createElement("div");

host.style.cssText = `
    position: fixed;
    inset: 0;
    z-index: 99999999;
    pointer-events: none;
`;

document.body.appendChild(host);

// Set mode to open to allow JavaScript to affect it
const shadow = host.attachShadow({mode: 'open'});

// load the font from the link with the name "Nunito"
const font = new FontFace('Nunito', 'url(https://fonts.gstatic.com/s/nunito/v26/XRXV3I6Li01BKofINeaB.woff2)');
// AD refutes top-level await statements, so wrap it in an async IIFE
(async () => {
    try {
        await font.load();
        document.fonts.add(font);
    } catch (e) {
        console.error('No nunito for you!', e);
    }
})();

/*
    from here the styles and innerHTML are inlined.
    creating a nested HTML is annoying to get working
    and trying to add to the base game files is also annoying.

    The work around is to just inject it with JavaScript
*/
const style = document.createElement("style");

style.textContent = `
    #root {
        --border-style: 1px solid #df5050;
        &.active {
            --border-style: 1px solid rgb(27, 187, 54);
        }
        --text-color: rgb(27, 187, 54);
        --button-bg: rgb(18,18,18);
        --button-bg-hover: rgb(27, 187, 54);
        --button-text: white;
        --button-text-hover: rgb(18,18,18);
        --text-main: white;
        --font-family: monospace Typewriter;
        --root-bg: rgba(18,18,18,1);
        --table-bg-odd: rgba(18,18,18,1);
        --table-bg-even: #df50504d;
    }
    #root.alt {
        --border-style: 2px solid #F2B8D0;
        &.active {
            --border-style: 2px solid #8CCFFF;
        }
        --text-color: #46b4d0;
        --button-bg: #F8DCE9;
        --button-bg-hover: #A9D8F7;
        --button-text: #ad73ff;
        --button-text-hover: #FFFFFF;
        --text-main: #ad73ff;
        --font-family: 'Nunito', 'monospace Typewriter';
        --root-bg: rgba(232, 245, 255, 1);
        --table-bg-odd: #E8F5FF;
        --table-bg-even: #ffcefd;
    }
    #root {
        border: var(--border-style);
        font-family: var(--font-family);
        position: fixed;
        left: 20px;
        top: 20px;
        width: 400px;
        height: 280px;
        display: grid;
        grid-template-rows: 48px minmax(0, 1fr) 20px;
        border-radius: 5px;
        overflow: hidden;
        white-space: nowrap;
        color: white;
        background: var(--root-bg);
        pointer-events: auto;
        user-select: text;
    }

    #root > *:not(#footer) {
        margin: 5px;
    }

    #headwrapper {
        width: 100%;
        height: 100%;
        display: grid;
        grid-template-columns: 1fr auto 1fr;   
    }

    header {
        color: var(--text-color);
        cursor: move;
        user-select: none;
        padding-left: 15px;
        font-size: 14px;
        box-sizing: border-box;
    }

    #title {
        grid-column: 2;
        justify-self: center;
        text-align: center;
        width: max-content;
    }

    tri-group {
        grid-column: 3;
        display: flex;
        justify-self: end;
        width: fit-content;
        align-self: start;
    }

    tri-group > * {
        user-select: none;
        cursor: pointer;
        align-self: flex-start;
        min-height: 20px;
        min-width: 25px;
        display: flex;
        justify-content: center;
    }

    tri-group > span:hover {
        background-color: rgba(240, 240, 240, .5);
    }

    tri-group > span.alt:hover {
        background-color: rgba(240, 0, 0, .8);
    }

    section-header {
        display: inline;
        font-weight: bold;
    }

    #table {
        width: 100%;
        table-layout: fixed;
        padding-left: 10px;
        padding-right: 10px;
        border-spacing: 0;
    }

    td {
        color: var(--text-main);
        text-align: left;
        padding: 7px 0 7px 8px;
        overflow: hidden;
    }

    tr:nth-child(even) {
        background-color: var(--table-bg-even);
    }

    tr:nth-child(odd) {
        background-color: var(--table-bg-odd);
    }

    [data-tab="1"] {
        width: 100%;
        height: 100%;
    }

    #resizewrapper {
        display: flex;
        justify-content: flex-end;
        align-items: flex-end;
        box-sizing: border-box;
    }

    #resize {
        user-select: none;
        cursor: nwse-resize;
        color: var(--text-color);
        margin-top: 6px;
    }

    #tab {
        justify-self: center;
        color: var(--text-color);
        display: flex;
        width: fit-content;
        gap: 20px;
        padding-bottom: 4px;
    }     

    #tab > * {
        cursor: pointer;
        user-select: none;
        border: var(--border-style);
        background-color: var(--button-bg);
        color: var(--button-text);
        padding: 0 6px 0 6px;
        border-radius: 4px;
    }

    #tab > *:hover {
        background-color: var(--button-bg-hover);
        color: var(--button-text-hover);
    }

    #tab > *.active {
        background-color: var(--button-bg-hover) !important;
        color: var(--button-text-hover) !important;
    }

    textarea {
        resize: none;
        margin-top: 15px;
        padding: 10px;
        background-color: var(--root-bg);
        border: var(--border-style);
        border-radius: 4px;
        color: var(--text-main);
        font-size: 12px;
        width: 75%;
        font-size: 16px;
        height: 2rem;
    }
    .label {
        color: var(--button-text);
    }

    #startTAS {
        user-select: none;
        justify-self: center;
        border: var(--border-style);
        border-radius: 4px;
        color: var(--button-text);
        background-color: var(--button-bg);
        margin: 10px;
        padding: 3px;
        width: fit-content;
        &:hover {
            background: var(--button-bg-hover);
            color: var(--button-text-hover)
        }
    }
    #stopTAS {
        user-select: none;
        justify-self: center;
        border: var(--border-style);
        border-radius: 4px;
        color: var(--button-text);
        background-color: var(--button-bg);
        margin: 10px;
        padding: 3px;
        width: fit-content;
        &:hover {
            background: var(--button-bg-hover);
            color: var(--button-text-hover)
        }
    }
    #startTAS:hover {
        background: var(--button-bg-hover);
        color: var(--button-text-hover);
    }
`;
// append the style to the shadow DOM
shadow.appendChild(style);
// create a root for the Shadow DOM
const root = shadow.appendChild(document.createElement("div"));
root.id = 'root';

root.innerHTML = `
    <header id="header">
        <div id='headwrapper'>
            <section-header id="title"> Advanced Layout (for) Easy eXecution </section-header>

            <tri-group>

                <span id='minimize'>—</span>
                <span id='restore'>▢</span>
                <span class='alt' id='close' style='font-size: 20px; line-height: 1;'>×</span>

            </tri-group>
        </div>
    </header>

    <div id="body">

        <div id='tab'>
            <div id="tab1" class="active">Diagnostics</div>
            <div id="tab2">TAS GUI</div>
        </div>

        <div data-tab="0">
            <table id="table">
                <tbody>
                </tbody>
            </table>
        </div>

        <div data-tab="1" hidden>
            <div id="contentwrapper">
            
                <div><textarea spellcheck='false' type='text' placeholder='Enter expression for the paths' id='enterpath'></textarea></div>
                <div><textarea spellcheck='false' type='text' placeholder='Enter expression for the save' id='entersave'></textarea></div>

                <div id='options'> 
                    <div class='label'> With TAS Diagnostics? <input type='checkbox' id='o1'></div>
                    <div class='label'> As segments? <input type='checkbox' id='o2'></div>
                </div>
                <br>
                <span id="startTAS"> Start TAS </span>
                <span id="stopTAS"> Stop TAS </span>

            </div>
        </div>

    </div>
    <div id="footer">
        <div id='resizewrapper'>
            <div id="resize">◢</div>
        </div>
    </div>
`;
window.root = root;

// jQuery style naming.
const $ = ( id ) => root.querySelector( `#${id}` );
const $$ = ( role, val ) => root.querySelector( `[data-${role}="${val}"]` );

$('startTAS').addEventListener("click", () => {

    const paths = $('enterpath');
    const save = $('entersave');

    let val1, val2;
    try {

        val1 = eval(paths.value);
        val2 = eval(save.value);
        
        if (!Array.isArray(val1)) {
            val1 = [val1];
        }

        if ($('o1').checked) {
            TAS.variables.debug |= 2;
        }

        $('o2').checked 
            ? TAS.getSegments( val1, val2 )
            : TAS.prepareAndStart( val1, val2 );

        root.classList.add('active');

    } catch(e) {
        root.classList.remove('active'); 
        alert(e);
    }
});

$('stopTAS').addEventListener("click", () => {
    TAS.pause();

    clearInterval(TAS.intervals["segmentIntervals"]);

    root.classList.remove('active');
})

const close = $('close');

close.addEventListener('click', () => {
    root.hidden = true;
    root.style.display = 'none';
});


// resize to fullscreen 
const restore = $('restore');

let restore_start = null;
let restore_maximized = false;

restore.addEventListener('click', () => {
    if ( !restore_maximized ) {

        restore_start = root.getBoundingClientRect();

        root.style.left = '0px';
        root.style.top = '0px';
        root.style.width = '100%';
        root.style.height = '100%';

        restore.textContent = '❐';
        root.style.borderRadius = '0px';

        header.style.setProperty('cursor', 'default', 'important');

        restore_maximized = true;

    } else {

        root.style.left = restore_start.x + 'px';
        root.style.top = restore_start.y + 'px'
        root.style.width = restore_start.width + 'px';
        root.style.height = restore_start.height + 'px';

        restore.textContent = '▢';

        header.style.removeProperty('cursor');

        root.style.borderRadius = '';
        
        restore_maximized = false;

    }
});

// minimize tab
const minimize = $('minimize');

let minimize_minimized = false;
let minimize_start = null;

minimize.addEventListener('click', () => {
    if ( restore_maximized ) return;
    if ( !minimize_minimized ) {

        minimize_start = root.getBoundingClientRect();

        root.style.height = header.getBoundingClientRect().height + 'px';

        body.hidden = true;
        footer.hidden = true;
        minimize_minimized = true;
    } else {

        root.style.height = minimize_start.height + 'px';

        body.hidden = false;
        footer.hidden = false;
        minimize_minimized = false;
    }
});


// draggable header
const header = $('header');

let header_dragging = false;
let header_val = null;
let pointerid = null;

header.addEventListener('pointerup', () => {
    if ( !header_dragging ) return;

    header.releasePointerCapture(pointerid);

    header_dragging = false;
});

header.addEventListener('pointercancel', () => {
    if ( !header_dragging ) return;

    header.releasePointerCapture(pointerid);

    header_dragging = false;
});

header.addEventListener('pointerdown', e => {
    if ( restore_maximized || e.button !== 0 || e.target.tagName == "SPAN" ) return;

    header_dragging = true;

    header_val = {
        x: e.clientX,
        y: e.clientY
    };

    header_val['left'] = root.offsetLeft;
    header_val['top'] = root.offsetTop;
    pointerid = e.pointerId;

    header.setPointerCapture(e.pointerId); 
});

header.addEventListener('pointermove', e => {
    if ( !header_dragging || restore_maximized ) return;

    root.style.left = header_val.left + e.clientX - header_val.x + 'px';
    root.style.top = header_val.top + e.clientY - header_val.y + 'px';
});

// Resize
const resize = $('resize');

let resize_resizing = false;
let resize_pointerid = null;
let base = null;

resize.addEventListener('pointerup', () => {
    if ( !resize_resizing ) return;

    resize.releasePointerCapture(resize_pointerid);

    resize_resizing = false
});

resize.addEventListener('pointercancel', () => {
    if ( !resize_resizing ) return;

    resize.releasePointerCapture(resize_pointerid);

    resize_resizing = false
});

resize.addEventListener('pointerdown', e => {
    if (restore_maximized) return;

    base = root.getBoundingClientRect();
    base['x'] = e.clientX;
    base['y'] = e.clientY;
    
    resize_resizing = true
    resize_pointerid = e.pointerId;

    resize.setPointerCapture(e.pointerId);
});

document.addEventListener('pointermove', e => {
    if ( !resize_resizing || restore_maximized ) return;

    root.style.width = Math.max(400, base.width + (e.clientX - base['x'])) + 'px';
    root.style.height = Math.max(80, base.height + (e.clientY - base['y'])) + 'px';
});


const body = $('body');
const footer = $('footer');

// changing tabs
const tabs = {
    0: $('tab1'),
    1: $('tab2'),
}

const bodies = {
    0: $$('tab', 0),
    1: $$('tab', 1)
}

function changeTab(n) { 
    Object.values(tabs).forEach(el => el.classList.remove('active'));
    Object.values(bodies).forEach(el => el.hidden = true);

    tabs[n].classList.add('active');
    bodies[n].hidden = false;        
}

tabs[0].addEventListener('click', () => changeTab(0));
tabs[1].addEventListener('click', () => changeTab(1));

// quick function to append an element with it's textContent and id to the tablebody
const tablebody = $("table").children[0];

function appendToTable(title, id) {
    const tr = document.createElement("tr");
    const value = document.createElement("td");
    const td = document.createElement("td");

    tablebody.appendChild(tr);

    tr.appendChild(td);
    tr.appendChild(value);
    
    td.textContent = title;
    // gross inling up style
    value.style.paddingLeft = '10px';
    
    value.id = id;

    return value;
};

appendToTable("Cycle counter", 'cycle');
appendToTable("Instruction #", 'inst1');
appendToTable("Current instruction", 'inst2');
appendToTable("Current TAS path", 'inst3');
appendToTable("Galaxies", 'gala1');
appendToTable("Dimboosts", 'dimb1');
appendToTable("Current game time", 'time1');
appendToTable("TAS run time", 'time2');
const events = [
    'cycle',
    'inst1',
    'inst2',
    'gala1',
    'dimb1',
    'time1',
    'time2',
    'inst3'
];

// Event Bus
const registeredEvents = {};

function registerEvent(name, fn) {

    if (!( name in registeredEvents)) {
        registeredEvents[name] = [];
    }

    registeredEvents[name].push(fn);
}

const removeEvent = (name) =>
    delete registeredEvents[name];

export function dispatch(name, ...args) {
    for (const callback of registeredEvents[name] ?? []) {
        callback(...args);
    }
}
 
for (const node of events) {
    let el = $(node);

    el.textContent = "Awaiting signal";

    registerEvent(node, txt => el.textContent = txt);

    blinkyText(el, '...', 300);
};


// purely cosmetic. Had fun making it - Jade
function blinkyText(node, text, delay) {
    let base = node.textContent;

    function* iterate() {
        for (const char of text) {
            yield char;
        }
    }

    const checkTamper = () => 
        node.textContent.slice(0, base.length) != base

    let iterator = iterate(text);

    let id = setInterval(() => {
        if (checkTamper()) {
            clearInterval(id);
            return;
        }

        let { value, done } = iterator.next();

        if (done) {
            node.textContent = base;

            iterator = iterate(text);
        } else {
            node.textContent += value;
        }

    }, delay);

    return id;
};