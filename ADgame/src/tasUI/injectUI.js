
    const host = document.createElement("div");
    // it requires no pointer events otherwise it will capture all of them
    host.style.cssText = `
        position: fixed;
        inset: 0;
        z-index: 99999999;
        pointer-events: none;
    `;
    document.body.appendChild(host);
    // open mode means I can affect the internals with javascript
    const shadow = host.attachShadow({mode: 'open'});
    const style = document.createElement("style");
    /*
     * normally I would use inline styles, but it's getting so annoying to work with
     * that I might as well just create a style and change it's text content. 
     * it works better because it's easier to manipulate, and creating
     * class's are so much more easier.
     */
    // radial-gradient is soo nice to make the flat color look more lively
    // same with border-radius
    // with the downside of looking vibe coded, as Alex pointed out.
    style.textContent = `
      #root {
        border: 1px solid green;
        padding: 5px;
        font-family: monospace Typewriter;
        position: fixed;
        left: 20px;
        top: 20px;
        width: 380px;
        height: 280px;
        display: grid;
        grid-template-rows: 48px minmax(0, 1fr) 20px;
        overflow: hidden;
        white-space: nowrap;
        color: white;
        background: radial-gradient(ellipse at 5% 8%, rgba(50, 255, 105, .20), transparent 100%), #0c1220;
        border: 2px solid black;
        font: monospace;
        pointer-events: auto;
        user-select: text;
      }
      header {
        cursor: move;
        user-select: none;
        display: flex;
        justify-content: space-between;
        padding-left: 15px;
        padding-top: 5px;
        box-sizing: border-box;
      }
      tri-group {
        display: flex;
        flex-stretch: flex-start;
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
      tri-group > div:hover {
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
        border-spacing: 5px 6px;
      }
      td {
        color: white;
        text-align: left;
      }
      [data-tab="1"] {
        width: 100%;
        height: 100%;
      }
      #resize {
        display: flex;
        color: white;
        height: fit-content;
        width: fit-content;
        justify-self: flex-end;
        cursor: nwse-resize;
        margin-top: 12px;
      }
    `;
    shadow.appendChild(style);
    const root = document.createElement("div");
    root.id = 'root';
    shadow.appendChild(root);
    root.innerHTML = `
        <header id="header">
            <section-header id="title"> Advanced Layout (for) Easy eXecution </section-header>
            <tri-group>
                <span id='minimize' style='line-height: 1.1'>-</span>
                <span id='restore' style='line-height: 1.2;'>▢</span>
                <div id='close'>X</div>
            </tri-group>
        </header>
        <div id="body" data-tab="1">
            <section data-tab="1">
                <table id="table">
                    <tbody></tbody>
                </table>
            </section>
            <section data-tab="2" hidden></section>
        </div>
        <div id="footer">
            <div id='reizewrapper'>
                <div id="resize">◢</div>
            </div>
        </div>
    `;
    const $ = id => root.querySelector(`#${id}`);
    
    
    const close = $('close');
    close.addEventListener('click', () => {
        root.hidden = true;
        root.style.display = 'none';
    });
    
    const restore = $('restore');
    let restore_start = null;
    let restore_maximized = false;
    restore.addEventListener('click', () => {
        if (!restore_maximized) {
            restore_start = root.getBoundingClientRect();
    
            root.style.left = '0px';
            root.style.top = '0px';
            root.style.width = '100%';
            root.style.height = '100%';
    
            restore.textContent = '❐';
            root.style.borderRadius = '0px';
            restore_maximized = true;
        } else {
            root.style.left = restore_start.x + 'px';
            root.style.top = restore_start.y + 'px'
            root.style.width = restore_start.width + 'px';
            root.style.height = restore_start.height + 'px';
    
            restore.textContent = '▢';
            root.style.borderRadius = '';
            restore_maximized = false;
        };
    });
    
    const minimize = $('minimize');
    let minimize_minimized = false;
    let minimize_start = null;
    
    minimize.addEventListener('click', () => {
        if (restore_maximized) return;
        if (!minimize_minimized) {
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
    
    const header = $('header');
    
    let header_dragging = false;
    let header_val = null;
    let pointerid = null;
    header.addEventListener('pointerup', () => {
        if (!header_dragging) return
        header.releasePointerCapture(pointerid);
        header_dragging = false;
    })
    header.addEventListener('pointercancel', () => {
        if (!header_dragging) return; // this is so it doesn't error with the releasepointercapture
        header.releasePointerCapture(pointerid);
        header_dragging = false;
    });
    
    header.addEventListener('pointerdown', e => {
        if (restore_maximized || e.button !== 0 || e.target.tagName == "DIV" || e.target.tagName == "SPAN") return;
        header_dragging = true;
        header_val = {
            x: e.clientX,
            y: e.clientY
        };
        header_val['left'] = root.offsetLeft;
        header_val['top'] = root.offsetTop;
        /* I skimmed the article [https://blog.r0b.io/post/creating-drag-interactions-with-set-pointer-capture-in-java-script/] it was a big help, and apparently setPointerCapture is helpful
        */
        pointerid = e.pointerId;
        header.setPointerCapture(e.pointerId); 
    });
    
    header.addEventListener('pointermove', e => {
        if (!header_dragging || restore_maximized) return;
        root.style.left = header_val.left + e.clientX - header_val.x + 'px';
        root.style.top = header_val.top + e.clientY - header_val.y + 'px';
    });
    
    const resize = $('resize');
    let resize_resizing = false;
    let resize_pointerid = null;
    let base = null;
    
    resize.addEventListener('pointerup', () => {
        if (!resize_resizing) return;
        resize.releasePointerCapture(resize_pointerid);
        resize_resizing = false
    });
    resize.addEventListener('pointercancel', () => {
        if (!resize_resizing) return;
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
        if (!resize_resizing || restore_maximized) return;

        root.style.width = Math.max(400, base.width + (e.clientX - base['x'])) + 'px';
        root.style.height = Math.max(80, base.height + (e.clientY - base['y'])) + 'px';
    });
    
    const body = $('body');
    const footer = $('footer');
    
    const tablebody = $("table").children[0];
    function appendToTable(title, id) {
        const tr = document.createElement("tr");
        tablebody.appendChild(tr);
        
        const td = document.createElement("td");
        td.textContent = title;
        tr.appendChild(td);
        
        const value = document.createElement("td");
        value.style.paddingLeft = '10px';
        tr.appendChild(value);
        
        value.id = id;
        return value;
    };
    
    appendToTable("Cycle counter", 'cycle');
    appendToTable("Instruction #", 'inst1');
    appendToTable("Current instruction", 'inst2');
    appendToTable("Galaxies", 'gala1');
    appendToTable("Dimboosts", 'dimb1');
    appendToTable("Current game time", 'time1');
    appendToTable("Current real time", 'time2');
    const events = [
        'cycle',
        'inst1',
        'inst2',
        'gala1',
        'dimb1',
        'time1',
        'time2'
    ];
    const registeredEvents = {};
    function registerEvent(name, fn) {
        if (!(name in registeredEvents)) {
            registeredEvents[name] = [];
        };
        registeredEvents[name].push(fn);
    };
    function flushEvent(name) {
        return delete registeredEvents[name]
    };
    export function dispatch(name, ...args) {
        for (const callback of registeredEvents[name] ?? []) {
            callback(...args);
        };
    };
    for (const event of events) {
        let el = $(event);
        el.textContent = "Awaiting signal"
        registerEvent(event, txt => el.textContent = txt);
        blinkyText(el, '...', 300);
    };
    function blinkyText(node, text, delay) {
        let base = node.textContent;
        function* iterate() {
            for (const char of text) {
                yield char
            };
        };
        function checkTamper() {
            console.log(node.textContent.slice(0, base.length));
            return node.textContent.slice(0, base.length) != base
        }
        let iterator = iterate(text);
        let id = setInterval(() => {
            if (checkTamper()) {
                clearInterval(id);
                return;
            }
            let iter = iterator.next();
            if (iter.done) {
                node.textContent = base;
                iterator = iterate(text);
            } else {
                node.textContent += iter.value;
            }
        }, delay);
        return id;
    };