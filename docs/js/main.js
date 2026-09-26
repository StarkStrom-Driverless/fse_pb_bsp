(function () {
  "use strict";

  /* ---------- hero terminal typing ---------- */
  var term = document.getElementById("term");
  var script = [
    ["p", "$ source .venv/bin/activate"],
    ["p", "$ ./ss example_list"],
    ["o", "adc  can_rx  can_tx  can_tod  error  eth_udp  fm  fsm\ninput  iob  output  pid  printf  pwm  spi  uart"],
    ["p", "$ ./ss flash_example adc"],
    ["o", "building examples/adc.c ..."],
    ["o", "signing image (MCUboot) ..."],
    ["ok", "flashed via OpenOCD  ✓"]
  ];

  function esc(s) { return s.replace(/&/g, "&amp;").replace(/</g, "&lt;"); }

  function renderStatic() {
    term.innerHTML = script.map(function (l) {
      return '<span class="' + l[0] + '">' + esc(l[1]) + "</span>";
    }).join("\n") + '\n<span class="p">$ </span><span class="cur"></span>';
  }

  function play() {
    var i = 0, out = "";
    function line() {
      if (i >= script.length) {
        term.innerHTML = out + '<span class="p">$ </span><span class="cur"></span>';
        return;
      }
      var kind = script[i][0], text = script[i][1], n = 0;
      var typed = kind === "p";
      (function tick() {
        n = typed ? n + 1 : text.length;
        term.innerHTML = out + '<span class="' + kind + '">' + esc(text.slice(0, n)) + '</span><span class="cur"></span>';
        if (n < text.length) return setTimeout(tick, 28);
        out += '<span class="' + kind + '">' + esc(text) + "</span>\n";
        i++;
        setTimeout(line, typed ? 350 : 220);
      })();
    }
    line();
  }

  if (window.matchMedia("(prefers-reduced-motion: reduce)").matches) renderStatic(); else play();

  /* ---------- tabs ---------- */
  document.querySelectorAll("[data-tabs]").forEach(function (root) {
    var tabs = root.querySelectorAll("[data-tab]");
    var panels = root.querySelectorAll("[data-panel]");
    tabs.forEach(function (t) {
      t.addEventListener("click", function () {
        tabs.forEach(function (x) { x.setAttribute("aria-selected", x === t); });
        panels.forEach(function (p) { p.hidden = p.getAttribute("data-panel") !== t.getAttribute("data-tab"); });
      });
    });
  });

  /* ---------- copy buttons ---------- */
  document.querySelectorAll("pre[data-copy]").forEach(function (pre) {
    var b = document.createElement("button");
    b.className = "copy"; b.type = "button"; b.textContent = "kopieren";
    b.addEventListener("click", function () {
      var txt = pre.querySelector("code").innerText;
      var done = function () { b.textContent = "kopiert ✓"; b.classList.add("done"); setTimeout(function () { b.textContent = "kopieren"; b.classList.remove("done"); }, 1400); };
      if (navigator.clipboard) navigator.clipboard.writeText(txt).then(done, function () {});
    });
    pre.appendChild(b);
  });

  /* ---------- stack diagram ---------- */
  var info = {
    mcu: ["STM32F405RTGE", "Das Fundament. Beide Stacks laufen auf demselben Chip."],
    zephyr: ["Zephyr", "Embedded-Betriebssystem, auf dem der Bootloader aufbaut. Eigenes Repo: fse_pb_bootloader."],
    mcuboot: ["MCUboot", "Sicherer Bootloader. Prüft signierte Images in Slot 1 und tauscht sie gegen Slot 0."],
    ocm3: ["libopencm3", "Low-Level-Konfigurationsbibliothek für verschiedene Mikrocontroller-Plattformen."],
    rtos: ["FreeRTOS", "Das Echtzeitbetriebssystem. fse_pb_bsp kapselt seine Nutzung (Tasks, Delays, Start)."],
    bsp: ["fse_pb_bsp", "Die eigene Abstraktionsschicht für die Controller-Konfiguration: GPIO, PWM, ADC, CAN, SPI, UART und mehr."],
    app: ["application", "Deine eigentliche Steuergeräte-Software, ohne Register- oder RTOS-Details."]
  };
  var box = document.getElementById("stackInfo");
  var layers = document.querySelectorAll(".lay");
  function show(el) {
    var d = info[el.dataset.k];
    layers.forEach(function (l) { l.classList.toggle("on", l === el); });
    box.innerHTML = "<h3></h3><p></p>";
    box.firstChild.textContent = d[0];
    box.lastChild.textContent = d[1];
  }
  layers.forEach(function (l) {
    l.tabIndex = 0;
    l.addEventListener("click", function () { show(l); });
    l.addEventListener("mouseenter", function () { show(l); });
    l.addEventListener("keydown", function (e) { if (e.key === "Enter" || e.key === " ") { e.preventDefault(); show(l); } });
  });

  /* ---------- pin explorer ---------- */
  // [pin, gpio, pwm, can, spi1, spi2, spi3, uart, adc]
  var T = 1;
  var pins = [
    ["PA0",T,T,"","","","","",T], ["PA1",T,T,"","","","","",T],
    ["PA2",T,T,"","","","","UART2_Tx",T], ["PA3",T,T,"","","","","UART2_Rx",T],
    ["PA4",T,"","","SPI1_NSS","","SPI3_NSS","",T], ["PA5",T,T,"","SPI1_SCK","","","",T],
    ["PA6",T,T,"","SPI1_MISO","","","",T], ["PA7",T,T,"","SPI1_MOSI","","","",T],
    ["PA8",T,T,"","","","","",""], ["PA9",T,T,"","","","","UART1_Rx",""],
    ["PA10",T,T,"","","","","UART1_Tx",""], ["PA11",T,T,"","","","","",""],
    ["PA12",T,"","","","","","",""], ["PA13","","","","","","","",""],
    ["PA14","","","","","","","",""], ["PA15",T,T,"","SPI1_NSS","","SPI3_NSS","",""],

    ["PB0",T,T,"","","","","",T], ["PB1",T,T,"","","","","",T], ["PB2","","","","","","","",""],
    ["PB3","","","","SPI1_SCK","","SPI3_SCK","",""], ["PB4","","","STB2","SPI1_MISO","","SPI3_MISO","",""],
    ["PB5","","","RX2","SPI1_MOSI","","SPI3_MOSI","",""], ["PB6","","","TX2","","","","",""],
    ["PB7","","","STB1","","","","",""], ["PB8","","","RX1","","","","",""],
    ["PB9","","","TX1","","SPI2_NSS","","",""],
    ["PB10",T,T,"","","SPI2_SCK","","UART3_Rx - UART4_Rx",""], ["PB11",T,T,"","","","","UART3_Tx",""],
    ["PB12",T,"","","","SPI2_NSS","","",""], ["PB13",T,"","","","SPI2_SCK","","",""],
    ["PB14",T,T,"","","SPI2_MISO","","",""], ["PB15",T,T,"","","SPI2_MOSI","","",""],

    ["PC0","","","","","","","",""], ["PC1","","","","","","","",""],
    ["PC2",T,"","","","","","",T], ["PC3",T,"","","","","","",T],
    ["PC4","","","","","","","",""], ["PC5","","","","","","","",""],
    ["PC6",T,T,"","","","","UART6_Rx",""], ["PC7",T,T,"","","","","UART6_Rx",""],
    ["PC8",T,T,"","","","","",""], ["PC9",T,T,"","","","","",""],
    ["PC10",T,"","","","","SPI3_SCK","",""], ["PC11",T,"","","","","SPI3_MISO","UART4_Tx",""],
    ["PC12",T,"","","","","SPI3_MOSI","",""], ["PC13",T,"","","","","","",""],
    ["PC14",T,"","","","","","",""], ["PC15",T,"","","","","","",""]
  ];
  var cols = ["GPIO", "PWM", "CAN", "SPI1", "SPI2", "SPI3", "UART", "ADC"];
  var state = { q: "", fn: null, port: "all" };

  var tbody = document.querySelector("#pinTable tbody");
  var count = document.getElementById("pinCount");
  var fWrap = document.getElementById("pinFilters");
  var pWrap = document.getElementById("pinPorts");

  function mkBtns(wrap, items, key) {
    items.forEach(function (it) {
      var b = document.createElement("button");
      b.type = "button"; b.textContent = it.label; b.setAttribute("aria-pressed", "false");
      b.addEventListener("click", function () {
        var on = state[key] === it.value;
        state[key] = (key === "port") ? it.value : (on ? null : it.value);
        wrap.querySelectorAll("button").forEach(function (x) { x.setAttribute("aria-pressed", "false"); });
        if (key === "port" || !on) b.setAttribute("aria-pressed", "true");
        render();
      });
      wrap.appendChild(b);
    });
  }
  mkBtns(fWrap, cols.map(function (c, i) { return { label: c, value: i + 1 }; }), "fn");
  mkBtns(pWrap, [{ label: "alle", value: "all" }, { label: "A", value: "PA" }, { label: "B", value: "PB" }, { label: "C", value: "PC" }], "port");
  pWrap.firstChild.setAttribute("aria-pressed", "true");

  function hl(text, q) {
    if (!q || !text) return esc(String(text));
    var i = text.toLowerCase().indexOf(q);
    if (i < 0) return esc(text);
    return esc(text.slice(0, i)) + "<mark>" + esc(text.slice(i, i + q.length)) + "</mark>" + esc(text.slice(i + q.length));
  }

  function render() {
    var q = state.q.trim().toLowerCase();
    var rows = pins.filter(function (p) {
      if (state.port !== "all" && p[0].indexOf(state.port) !== 0) return false;
      if (state.fn && !p[state.fn]) return false;
      if (q && p.filter(function (v) { return v !== 1; }).join(" ").toLowerCase().indexOf(q) < 0) return false;
      return true;
    });
    tbody.innerHTML = rows.map(function (p) {
      var any = p.slice(1).some(Boolean);
      var tds = p.slice(1).map(function (v) {
        if (v === 1) return '<td class="yes">●</td>';
        return v ? '<td class="sig">' + hl(v, q) + "</td>" : "<td></td>";
      }).join("");
      return '<tr class="' + (any ? "" : "none") + '"><td>' + hl(p[0], q) + "</td>" + tds + "</tr>";
    }).join("") || '<tr><td colspan="9" style="color:var(--muted)">Keine Treffer.</td></tr>';
    count.textContent = rows.length + " von " + pins.length + " Pins";
  }

  document.getElementById("pinSearch").addEventListener("input", function (e) { state.q = e.target.value; render(); });
  render();
})();
