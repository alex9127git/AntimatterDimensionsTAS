const host = document.createElement("div");
host.id = 'shadowDOM';
host.style.cssText = `
    position: fixed;
    inset: 0;
    z-index: 2147483647;
    pointer-events: none;
`;
document.body.appendChild(host);
const shadow = host.attachShadow({mode: 'open'});
/* ----- container for everything ------ */
export const container = createChildElement(shadow, 'div', 'container', {
    style: 'display: grid; grid-column: 1/3; grid-template-columns: 1fr; grid-template-rows: 40px minmax(0, 1fr) 30px; position: absolute; background-color: white; width: 300px; height: 200px; user-select: none; border: 1px solid black; box-sizing: border-box; left: 0px; top: 0px; z-index: 99999; color: black; pointer-events: auto;'
});
const $ = e => container.querySelector(`#${e}`);
/* ------ append ALL of the containers children here ------ */
const header = createChildElement(container, 'div', 'header', {
    textContent: "Advanced Layout (for) Easy eXecution",
    style: 'display: flex; justify-content: space-between; background-color: pink; width: 100%; height: 30px; margin: 0; padding: 0; cursor: move; overflow: hidden; font-size: 12px;',
    _isMoving: false,
    _startX: 0,
    _startY: 0
});
// wrapper
const trigroup = createChildElement(header, 'div', 'grouper', {
    style: 'display: flex; flex-direction: row; margin: 0; padding: 0;'
});
// semantic style that all of the trigroups elements have
let trigroup_style = 'display: flex; justify-content: center; align-text: center; width: 15px; height: 100%; font-size: 20px; padding-right: 5px; padding-left: 5px; user-select: none; cursor: pointer;';
const restore = createChildElement(trigroup, 'div', 'close', {
    textContent: '□',
    style: trigroup_style,
    _startX: null,
    _startY: null,
    _startW: null,
    _startH: null,
    _resized: false
});
const minimize = createChildElement(trigroup, 'div', 'minimize', {
    textContent: '_',
    style: 'line-height: .2;' + trigroup_style,
    _minimized: false,
    _startH: '200px'
});
const close = createChildElement(trigroup, 'div', 'close', {
    textContent: 'x',
    style: trigroup_style
});

const body = createChildElement(container, 'div', 'body', {
    style: 'display: flex; flex-direction: column; width: 100%; height: fit-content; overflow: hidden; font-family: monospace;'
});
const footer = createChildElement(container, 'div', 'footer', {
    style: 'display: flex; justify-content: flex-end; background-color: pink; height: 30px;'
});
// a wrapper for the resize
const monogroup = createChildElement(footer, 'div', 'digroup', {
    style: 'width: 10px; background-color: inherit;display: flex; justify-content: flex-end; flex-direction: column; padding: 0; margin: 0;'
});
const resize = createChildElement(monogroup, 'div', 'resize', {
    textContent: '◢',
    style: 'line-height: .7; cursor: se-resize;',
    _startX: null,
    _startY: null,
    _startW: container.offsetWidth,
    _startH: container.offsetHeight,
    _resizing: false
});
// when hovering these elements, give them a certain color
registerHoverEvent(close, 'rgba(255,0,0,.9)');
registerHoverEvent(minimize, 'rgba(245, 245, 245, .55)');
registerHoverEvent(restore, 'rgba(245, 245, 245, .55)');

close.addEventListener("click", () => {
    container.style.display = 'none';
    container.hidden = true;
});
/* ------ full screen ------ */
restore.addEventListener("click", () => {
    if (restore._resized === false) {
        header.style.cursor = 'default';
        
        restore._startX = container.offsetLeft + 'px';
        restore._startY = container.offsetTop + 'px';
        restore._startW = container.offsetWidth + 'px';
        restore._startH = container.offsetHeight + 'px';
        
        container.style.left = '0px';
        container.style.top = '0px';
        container.style.width = '100%';
        container.style.height = '100%';

        restore._resized = true;
        restore.textContent = '■';
    } else if (restore._resized === true) {
        header.style.cursor = 'move';
        
        container.style.left = restore._startX;
        container.style.top = restore._startY;
        container.style.width = restore._startW
        container.style.height = restore._startH;
        
        restore._resized = false;
        restore.textContent = '□';
    };
});
/* ------ minimize ------ */
minimize.addEventListener("click", () => {
    if (restore._resized) return;

    if (minimize._minimized === false) {
        minimize._startH = container.style.height;
        body.style.display = 'none';
        footer.style.display = 'none';
        container.style.height = '30px';
        minimize._minimized = true;
        
        container.style.height = header.style.height;
    } else if (minimize._minimized === true) {
        body.style.display = 'flex';
        footer.style.display = 'flex';
        container.style.height = minimize._startH;
        minimize._minimized = false;
    }
});
/* ------ dragging ------ */
header.addEventListener("mousedown", e => {
    if (e.target !== header) return;
    header._isMoving = true
    header._startX = e.clientX - container.offsetLeft;
    header._startY = e.clientY - container.offsetTop;
}); document.addEventListener("mouseup", ()=> {
    header._isMoving = false
}); document.addEventListener("mousemove", e => {
    if (header._isMoving === false || restore._resized) return;
    container.style.left = Math.max(0, (e.clientX - header._startX)) + 'px';
    container.style.top = Math.max(0, (e.clientY - header._startY)) + 'px' ;
});
/* ------ resize ------ */
resize.addEventListener("pointerdown", e => {
    resize._startX = e.clientX;
    resize._startY = e.clientY;
    resize._startW = container.offsetWidth;
    resize._startH = container.offsetHeight;
    resize._resizing = true;
}); document.addEventListener("pointerup", () => {
    resize._resizing = false;
}); document.addEventListener("mousemove", e => {
    if (!resize._resizing || restore._resized) return;
    container.style.width = Math.max(resize._startW - (resize._startX - e.clientX), 200) + 'px';
    container.style.height = Math.max(resize._startH - (resize._startY - e.clientY), 100) + 'px';
});
/* ---------------------- */

// diagnostics
const b = createChildElement(body, 'div', 'cycle', {
    textContent: 'Cycle counter:'
});
createChildElement(body, 'div', 'inst1', {
    textContent: 'Instruction#:'
});
createChildElement(body, 'div', 'inst2', {
    textContent: 'Current instruction:'
});
createChildElement(body, 'div', 'gala1', {
    textContent: 'Current galaxies:'
});
createChildElement(body, 'div', 'dimb1', {
    textContent: 'Current dimboosts:'
});
createChildElement(body, 'div', 'time1', {
    textContent: 'Current rt in run:'
});
createChildElement(body, 'div', 'time2', {
    textContent: 'Current gt in run:'
});

container.__is_dependency__ = true;



export function registerHoverEvent(node, color) {
    node.addEventListener("mouseover", () => {
        node.style.backgroundColor = color;
    }); node.addEventListener("mouseout", () => {
        node.style.backgroundColor = 'inherit';
    });
};

export function createChildElement(parent, type, id, meta) {
    return parent.appendChild(Object.assign(document.createElement(type), {id}, meta));
};

const registeredEvents = {};
function registerEvent(name, fn) {
    if (!(name in registeredEvents)) {
        registeredEvents[name] = [];
    };
    registeredEvents[name].push(fn);
};
export function flushEvent(name) {
    return delete registeredEvents[name]
};
export function dispatch(name, ...args) {
    for (const callback of registeredEvents[name]) {
        callback(...args);
    };
};
window['dispatch'] = dispatch;
export const events = [
    'cycle',
    'inst1',
    'inst2',
    'gala1',
    'dimb1',
    'time1',
    'time2'
];
const basetext = {};
events.forEach(name => {
    basetext[name] = $(name).textContent +' ';
});
events.forEach(name => {
    registerEvent(name, data => {
        $(name).textContent = basetext[name] + data;
    });
});