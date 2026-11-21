/*
  File   : web/app.js
  Part   : C (Foundations, Interface, Web)
  Purpose: Logic of the page. Every button sends ONE command line (names, never
           ids) to the bridge with POST /api and shows the JSON reply.
           The page never calculates a route itself.
  Flow   : button click -> api("ROUTE_DIST|Saddar|Clifton") -> bridge -> C++ engine
           -> one JSON line back -> showRoute() + draw() highlight it on the map.
*/
const $ = id => document.getElementById(id);
const COLORS = { road: '#9aabb8', route: '#0b8a8a', blocked: '#c4362b', mst: '#6a4ca3', ink: '#14212c' };

let map = { locations: [], roads: [] };      // last GET_MAP reply
let hl = { segs: [], nodes: [], color: COLORS.route };   // what is highlighted on the map
let lastRoute = null;                        // last successful ROUTE_* command, repeated after a closure change

// ------------------------------------------------------------------ helpers
// Sends one command to the bridge. Always returns an object with "ok".
async function api(cmd) {
  try {
    const res = await fetch('/api', { method: 'POST', body: cmd });
    return await res.json();
  } catch (e) {
    return { ok: false, error: 'Cannot reach the server. Is bridge/server.py running?' };
  }
}

// Shows a message at the top: red for errors, green for confirmations.
function say(text, ok) {
  const m = $('msg');
  m.textContent = text || '';
  m.className = ok ? 'ok' : 'err';
  m.hidden = !text;
}

// Runs a command. On failure shows the error (red) and returns null.
async function run(cmd) {
  const r = await api(cmd);
  if (!r.ok) { say(r.error || 'Unknown error'); return null; }
  say('');
  return r;
}

// Runs an edit command, shows its confirmation and reloads the map.
async function edit(cmd) {
  const r = await run(cmd);
  if (r) { say(r.message, true); await loadMap(); }
}

// Small DOM helper (textContent only, so names can never inject HTML).
function el(tag, text, cls) {
  const e = document.createElement(tag);
  if (text !== undefined) e.textContent = text;
  if (cls) e.className = cls;
  return e;
}

const val = id => $(id).value.trim();

// ---------------------------------------------------------------- map data
async function loadMap() {
  const r = await run('GET_MAP');
  if (!r) return;
  map = r;
  fillSelects();
  draw();
}

// Refills every location dropdown, the road dropdown and the stop checklist.
function fillSelects() {
  document.querySelectorAll('select.loc').forEach(sel => {
    const keep = sel.value;
    sel.replaceChildren(...map.locations.map(l => new Option(l.name, l.name)));
    if (map.locations.some(l => l.name === keep)) sel.value = keep;
  });
  const rd = $('cRoad'), keepRoad = rd.value;
  rd.replaceChildren(...map.roads.map(x => new Option(x.a + ' to ' + x.b + (x.blocked ? ' (blocked)' : ''), x.a + '|' + x.b)));
  if (keepRoad) rd.value = keepRoad;
  const ticked = new Set([...document.querySelectorAll('#tStops input:checked')].map(i => i.value));
  $('tStops').replaceChildren(...map.locations.map(l => {
    const lab = el('label'), box = el('input');
    box.type = 'checkbox'; box.value = l.name; box.checked = ticked.has(l.name);
    lab.append(box, l.name);
    return lab;
  }));
  fillLocationForm();
}

// ------------------------------------------------------------------ drawing
function draw() {
  const c = $('map'), g = c.getContext('2d');
  g.clearRect(0, 0, c.width, c.height);
  const pos = {};
  map.locations.forEach(l => { pos[l.name] = l; });

  const line = (a, b, color, width, dash) => {
    const p = pos[a], q = pos[b];
    if (!p || !q) return;
    g.beginPath(); g.setLineDash(dash || []); g.strokeStyle = color; g.lineWidth = width;
    g.lineCap = 'round'; g.moveTo(p.x, p.y); g.lineTo(q.x, q.y); g.stroke();
  };
  map.roads.forEach(r => r.blocked ? line(r.a, r.b, COLORS.blocked, 3, [8, 6]) : line(r.a, r.b, COLORS.road, 2));
  hl.segs.forEach(s => line(s[0], s[1], hl.color, 6));
  g.setLineDash([]);

  g.font = '600 13px "Segoe UI", system-ui, sans-serif';
  g.lineJoin = 'round';
  map.locations.forEach(l => {
    const on = hl.nodes.includes(l.name);
    g.beginPath(); g.arc(l.x, l.y, on ? 8 : 6, 0, Math.PI * 2);
    g.fillStyle = on ? hl.color : '#fff'; g.fill();
    g.lineWidth = 3; g.strokeStyle = on ? '#fff' : COLORS.ink; g.stroke();
    if (on) { g.beginPath(); g.arc(l.x, l.y, 8, 0, Math.PI * 2); g.lineWidth = 2; g.strokeStyle = COLORS.ink; g.stroke(); }
    const right = l.x > 560;                     // keep labels on the canvas
    g.textAlign = right ? 'right' : 'left';
    const tx = l.x + (right ? -12 : 12), ty = l.y + 4;
    g.lineWidth = 4; g.strokeStyle = 'rgba(255,255,255,0.9)'; g.strokeText(l.name, tx, ty);   // white halo
    g.fillStyle = COLORS.ink; g.fillText(l.name, tx, ty);
  });
}

// Highlights a list of location names as a path.
function setPath(path, color) {
  hl = { segs: [], nodes: path || [], color: color || COLORS.route };
  for (let i = 0; i + 1 < hl.nodes.length; i++) hl.segs.push([hl.nodes[i], hl.nodes[i + 1]]);
  draw();
}

// Highlights MST roads ({a, b} objects).
function setEdges(edges) {
  hl = { segs: edges.map(e => [e.a, e.b]), nodes: [], color: COLORS.mst };
  draw();
}

// ----------------------------------------------------------------- results
// Fills box with a route: the path, then distance, time and number of roads.
function showRoute(box, r, extra) {
  box.replaceChildren();
  box.append(el('p', r.path.join(' → '), 'path'));
  const dl = el('dl');
  [['Distance', r.totalDist + ' km'], ['Time', r.totalTime + ' min'], ['Roads', r.hops]].forEach(([k, v]) => {
    const d = el('div'); d.append(el('dt', k), el('dd', String(v))); dl.append(d);
  });
  box.append(dl);
  if (extra) box.append(el('small', extra));
}

function showList(box, items) {
  box.replaceChildren();
  if (!items.length) { box.append(el('p', 'Nothing found.', 'hint')); return; }
  const ul = el('ul');
  items.forEach(t => ul.append(el('li', t)));
  box.append(ul);
}

// -------------------------------------------------------------------- tabs
function openTab(name) {
  document.querySelectorAll('.panel').forEach(p => { p.hidden = p.id !== name; });
  document.querySelectorAll('#tabs button').forEach(b => b.classList.toggle('on', b.dataset.tab === name));
  if (name === 'history') loadHistory();
}
$('tabs').addEventListener('click', e => { if (e.target.dataset.tab) openTab(e.target.dataset.tab); });

// ------------------------------------------------------------------- route
$('rGo').onclick = async () => {
  const cmd = `${$('rMode').value}|${val('rFrom')}|${val('rTo')}`;
  const r = await run(cmd);
  if (!r) { $('rOut').replaceChildren(); return; }
  lastRoute = cmd;
  showRoute($('rOut'), r);
  setPath(r.path);
};

// -------------------------------------------------------------- multi-stop
$('tGo').onclick = async () => {
  const stops = [...document.querySelectorAll('#tStops input:checked')].map(i => i.value);
  if (stops.length < 2 || stops.length > 6) { say('Pick between 2 and 6 stops.'); return; }
  const r = await run(`MULTI|${$('tMode').value}|${val('tFrom')}|${stops.join('|')}`);
  if (!r) { $('tOut').replaceChildren(); return; }
  showRoute($('tOut'), r, 'Visit order: ' + r.order.join(', '));
  setPath(r.path);
};

// -------------------------------------------------------------- alternatives
$('aGo').onclick = async () => {
  const r = await run(`ALTERNATES|${val('aFrom')}|${val('aTo')}|${$('aKey').value}`);
  const box = $('aOut');
  box.replaceChildren();
  if (!r) return;
  const ul = el('ul');
  r.routes.forEach((rt, i) => {
    const b = el('button'), li = el('li');
    b.append(el('span', `Route ${i + 1}: ${rt.totalDist} km, ${rt.totalTime} min, ${rt.hops} roads`), el('small', rt.path.join(' → ')));
    b.onclick = () => { ul.querySelectorAll('button').forEach(x => x.classList.remove('on')); b.classList.add('on'); setPath(rt.path); };
    li.append(b); ul.append(li);
  });
  box.append(ul);
  ul.querySelector('button').click();            // show the best one first
};

// ---------------------------------------------------------------- closures
async function closure(cmdName) {
  const road = val('cRoad');
  if (!road) { say('Choose a road first.'); return; }
  const r = await run(`${cmdName}|${road}`);
  if (!r) return;
  say(r.message, true);
  await // Start: load the map, and pre-select a different 'to' location so the first click already works.
loadMap().then(() => ['rTo', 'aTo', 'roB'].forEach(id => { if ($(id).options.length > 1) $(id).selectedIndex = 1; }));
  const box = $('cOut');
  box.replaceChildren();
  if (lastRoute) {                               // plan the last route again, avoiding blocked roads
    const again = await api(lastRoute);
    if (again.ok) { box.append(el('p', 'Route planned again:', 'hint')); showRoute(box, again); setPath(again.path); }
    else { box.append(el('p', again.error, 'hint')); setPath([]); }
  }
}
$('cBlock').onclick = () => closure('BLOCK');
$('cUnblock').onclick = () => closure('UNBLOCK');

// ---------------------------------------------------------------- map editor
// Copies the selected location's data into the form so it can be updated.
function fillLocationForm() {
  const l = map.locations.find(x => x.name === val('eLoc'));
  if (l) { $('eName').value = l.name; $('eX').value = l.x; $('eY').value = l.y; }
}
$('eLoc').onchange = fillLocationForm;
$('eAdd').onclick = () => edit(`ADD_LOCATION|${val('eName')}|${val('eX')}|${val('eY')}`);
$('eUpd').onclick = () => edit(`UPDATE_LOCATION|${val('eLoc')}|${val('eName')}|${val('eX')}|${val('eY')}`);
$('eDel').onclick = () => { if (confirm('Remove ' + val('eLoc') + ' and all its roads?')) edit(`REMOVE_LOCATION|${val('eLoc')}`); };
$('roAdd').onclick = () => edit(`ADD_ROAD|${val('roA')}|${val('roB')}|${val('roD')}|${val('roT')}`);
$('roUpd').onclick = () => edit(`UPDATE_ROAD|${val('roA')}|${val('roB')}|${val('roD')}|${val('roT')}`);
$('roDel').onclick = () => edit(`REMOVE_ROAD|${val('roA')}|${val('roB')}`);
$('eUndo').onclick = () => edit('UNDO');
$('eSave').onclick = async () => { const r = await run('SAVE'); if (r) say(r.message, true); };

// ------------------------------------------------------------------ search
function showNames(names) {
  const box = $('sOut');
  box.replaceChildren();
  if (!names.length) { box.append(el('p', 'No location starts with that text.', 'hint')); return; }
  const ul = el('ul');
  names.forEach(n => {
    const li = el('li'), b = el('button', n);
    b.onclick = () => { setPath([n]); };         // click a name to mark it on the map
    li.append(b); ul.append(li);
  });
  box.append(ul);
}
$('sGo').onclick = async () => { const r = await run('SEARCH|' + val('sText')); if (r) showNames(r.names); };
$('sAll').onclick = async () => { const r = await run('LIST_SORTED'); if (r) showNames(r.names); };

// ---------------------------------------------------------------- analysis
$('nConn').onclick = async () => {
  const r = await run('CONNECTIVITY');
  if (!r) return;
  const box = $('nOut');
  box.replaceChildren(el('p', r.connected ? 'Every location can reach every other one.' : `The map is split into ${r.components.length} separate groups:`, 'path'));
  if (!r.connected) {
    const ul = el('ul');
    r.components.forEach((c, i) => ul.append(el('li', `Group ${i + 1}: ${c.join(', ')}`)));
    box.append(ul);
  }
};
$('nMst').onclick = async () => {
  const r = await run('MST');
  if (!r) return;
  setEdges(r.edges);
  const box = $('nOut');
  box.replaceChildren(el('p', r.connected ? 'Cheapest set of roads that joins every location' : 'Cheapest roads inside each separate group', 'path'));
  const dl = el('dl'), d = el('div');
  d.append(el('dt', 'Total length'), el('dd', r.totalDist + ' km'));
  const d2 = el('div');
  d2.append(el('dt', 'Roads used'), el('dd', String(r.edges.length)));
  dl.append(d, d2); box.append(dl);
};

// ----------------------------------------------------------------- history
async function loadHistory() {
  const r = await run('HISTORY');
  if (r) showList($('hOut'), r.history);
}
$('hGo').onclick = loadHistory;

// ------------------------------------------------------------------- start
// Start: load the map, and pre-select a different 'to' location so the first click already works.
loadMap().then(() => ['rTo', 'aTo', 'roB'].forEach(id => { if ($(id).options.length > 1) $(id).selectedIndex = 1; }));
