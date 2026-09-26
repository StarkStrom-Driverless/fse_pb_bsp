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

  /* ---------- example internals ---------- */
  // node: [type, label, sub, arrowLabelBefore]  type: hw | isr | obj | api | task
  var EX = {
    can_rx: {
      sum: "CAN-Frames kommen im Interrupt an, landen je CAN-ID in einer Queue und werden von deinem Task abgeholt.",
      lanes: [
        { t: "Einmalig im Task: Queue öffnen", n: [
          ["task", "dein Task", "startet"],
          ["api", "ss_can_queue_add(1, 0x22, &queue)", "legt Queue an (3 Frames)", "ruft"],
          ["obj", "Zuordnung ID → Queue", "Hashmap pro CAN-Kanal", "trägt ein"]
        ]},
        { t: "Empfangen", n: [
          ["hw", "CAN-Bus", "Frame mit ID 0x22"],
          ["isr", "can1_rx0_isr", "liest Frame aus FIFO 0", "löst aus"],
          ["obj", "Queue-Suche nach std_id", "unbekannte IDs werden verworfen", "schlägt nach"],
          ["obj", "Queue ID 0x22", "3 × SS_CAN_FRAME", "xQueueSendFromISR"],
          ["api", "ss_can_queue_read(queue, &msg)", "ohne Warten, 1 = Frame da", "holt ab"],
          ["task", "dein Task", "msg.std_id, msg.dlc, msg.data[]"]
        ]}
      ],
      note: "Wird dieselbe ID mehrfach mit ss_can_queue_add angelegt, entstehen parallele Queues: der Interrupt legt eine Kopie des Frames in jede davon. Mit ss_can_queue_add_combined sammelt eine Queue mehrere IDs. Zwei Queues im Beispiel (0x23 und 0x24) werden getrennt gelesen."
    },
    can_tx: {
      sum: "Dein Task schreibt Frames in eine Sende-Queue. Ist eine Mailbox frei, geht der Frame sofort raus, sonst leert der TX-Interrupt die Queue.",
      lanes: [
        { t: "Senden", n: [
          ["task", "dein Task", "ss_can_frame_set_common / _set_signal"],
          ["api", "ss_can_send(1, &msg)", "false, wenn Queue voll", "schreibt"],
          ["obj", "TX-Queue", "bis zu 32 Frames", "xQueueSend"],
          ["api", "ss_can_tx_kick", "füllt freie Mailboxen", "sofort"],
          ["hw", "CAN-Mailboxen", "3 Stück"],
          ["hw", "CAN-Bus", "Frame ID 0x123"]
        ]},
        { t: "Wenn alle Mailboxen belegt waren", n: [
          ["hw", "Mailbox wird frei", "TX-Complete"],
          ["isr", "can1_tx_isr", "quittiert und holt nächsten Frame", "löst aus"],
          ["obj", "TX-Queue", "xQueueReceiveFromISR", "liest"],
          ["hw", "CAN-Mailbox", "→ Bus"]
        ]}
      ],
      note: "Ein Frame, der nicht in die Hardware passt, wird vorne in die Queue zurückgelegt, die Reihenfolge bleibt erhalten."
    },
    can_tod: {
      sum: "Time-out-Detection: Ein Zähler pro erwarteter CAN-ID läuft ab, solange keine Nachricht kommt.",
      lanes: [
        { t: "Nachricht kommt an (Reset)", n: [
          ["hw", "CAN-Bus", "ID 0x123"],
          ["isr", "can1_rx0_isr", "", ""],
          ["obj", "Queue ID 0x123", "", "xQueueSendFromISR"],
          ["task", "can_task", "ss_can_queue_read", "holt ab"],
          ["api", "ss_can_tod_update(1, 0x123)", "Zähler zurück auf Startwert", "ruft"]
        ]},
        { t: "Zyklisch prüfen", n: [
          ["task", "tod_task (alle 500 ms)", ""],
          ["api", "ss_can_tod_check()", "Zähler − 1, bei 0: Timeout", "ruft"],
          ["obj", "Zähler pro ID", "ss_can_tod_add(1, 0x123, 10)", "prüft"],
          ["task", "state_task", "Error-LED blinkt bei Timeout", "Flag"]
        ]}
      ]
    },
    adc: {
      sum: "Ein Pin wird per Funktionsaufruf gewandelt. Kein Interrupt, kein DMA: der Aufruf wartet, bis der Wert da ist.",
      lanes: [{ t: "Messen", n: [
        ["hw", "Pin (analog)", "z. B. PA0"],
        ["hw", "ADC-Peripherie", "Kanal aus Pin abgeleitet", "Wandlung"],
        ["api", "ss_adc_read(pin, &value)", "startet und wartet auf EOC", "pollt"],
        ["task", "dein Task", "uint16_t value"]
      ]}]
    },
    pwm: {
      sum: "Ein Timer erzeugt das Signal in Hardware, dein Task ändert nur den Tastgrad in Prozent.",
      lanes: [{ t: "Ausgeben", n: [
        ["task", "dein Task", "value 0..100"],
        ["api", "ss_pwm_write(pin, value)", "Init: ss_pwm_init(pin, 10000 Hz)", "ruft"],
        ["hw", "Timer Compare-Register", "Timer aus dem Pin abgeleitet"],
        ["hw", "Pin (PWM)", "Duty-Cycle"]
      ]}]
    },
    input: {
      sum: "Ein digitaler Eingang wird im Task abgefragt (Polling), optional mit Pull-up.",
      lanes: [{ t: "Lesen", n: [
        ["hw", "Pin", "SS_GPIO_MODE_INPUT_PU"],
        ["hw", "GPIO-Eingangsregister", ""],
        ["api", "ss_io_read(pin)", "liefert 0 oder 1", "liest"],
        ["task", "dein Task", ""]
      ]}]
    },
    output: {
      sum: "Ein digitaler Ausgang wird im Task gesetzt, gelöscht oder getoggelt.",
      lanes: [{ t: "Schreiben", n: [
        ["task", "dein Task", ""],
        ["api", "ss_io_write(pin, SS_GPIO_TOGGLE)", "Init: SS_GPIO_MODE_OUTPUT", "ruft"],
        ["hw", "GPIO-Ausgangsregister", ""],
        ["hw", "Pin", ""]
      ]}]
    },
    iob: {
      sum: "Ein Interrupt merkt sich eine Flanke, auch wenn der Task gerade schläft. Der Task holt das Ereignis später ab.",
      lanes: [
        { t: "Einmalig", n: [
          ["api", "ss_iob_add(pin, SS_GPIO_FALLING)", "wählt Flanke", ""],
          ["hw", "EXTI-Leitung", "aus der Pin-Nummer", "konfiguriert"]
        ]},
        { t: "Flanke", n: [
          ["hw", "Pin", "fallende Flanke"],
          ["isr", "exti*_isr", "quittiert Pending-Bit", "löst aus"],
          ["obj", "Flag pro Pin", "value = 1", "setzt"],
          ["api", "ss_iob_get(pin)", "liest und löscht das Flag", "liest"],
          ["task", "dein Task", "im Beispiel: 0 = ausgelöst"]
        ]}
      ]
    },
    fm: {
      sum: "Frequenzmessung: Ein Timer mit Input Capture stempelt jede Flanke, der Task liest das Ergebnis als float.",
      lanes: [{ t: "Messen", n: [
        ["hw", "Signal am Pin", "z. B. Raddrehzahl"],
        ["hw", "Timer Input Capture", "speichert Zählerstand je Flanke", "Flanke"],
        ["isr", "ss_fm_isr", "liest CCRx, quittiert Flag", "löst aus"],
        ["obj", "Zustand pro Pin", "Zeit zwischen Flanken", "aktualisiert"],
        ["api", "ss_fm_read(pin, &value)", "float", "liest"],
        ["task", "dein Task", ""]
      ]}]
    },
    spi: {
      sum: "SPI überträgt vollduplex: beim Senden der tx-Bytes kommen gleichzeitig rx-Bytes zurück.",
      lanes: [{ t: "Transfer", n: [
        ["task", "dein Task", "tx[4] → rx[4]"],
        ["api", "ss_spi_rxtx(1, rx, tx, 4)", "Init: ss_spi_init(1, 1 MHz, mode 0)", "ruft"],
        ["hw", "SPI1-Peripherie", ""],
        ["hw", "SCK / MOSI / MISO", "Pins laut Tabelle"]
      ]}]
    },
    uart: {
      sum: "Minimales Grundgerüst für die serielle Schnittstelle. So laufen Daten durch den UART-Treiber.",
      lanes: [
        { t: "Empfangen", n: [
          ["hw", "UART-Pin RX", ""],
          ["isr", "usartN_isr", "Byte gelesen", "löst aus"],
          ["obj", "rx.queue", "Bytes", "xQueueSendFromISR"],
          ["api", "ss_uart_read(if, &byte)", "", "holt ab"],
          ["task", "dein Task", ""]
        ]},
        { t: "Senden", n: [
          ["task", "dein Task", ""],
          ["api", "ss_uart_send / _send_str", "", "ruft"],
          ["obj", "tx.queue", "Bytes", "schreibt"],
          ["isr", "usartN_isr", "TX-Register leer", "xQueueReceiveFromISR"],
          ["hw", "UART-Pin TX", ""]
        ]}
      ]
    },
    printf: {
      sum: "Formatierte Ausgabe über UART. Formatiert im Task, Übertragung übernimmt der UART-Treiber.",
      lanes: [{ t: "Ausgeben", n: [
        ["task", "dein Task", "ss_printf(4, \"ADC: %d\", v)"],
        ["api", "ss_printf", "formatiert in Puffer", "ruft"],
        ["api", "ss_uart_send_str", "", "reicht weiter"],
        ["obj", "tx.queue", "", "schreibt"],
        ["isr", "uart4_isr", "", "leert"],
        ["hw", "UART-Pin TX", "Terminal"]
      ]}]
    },
    error: {
      sum: "Fehlerbehandlung: SS_ERROR_ASSERT und SS_ERROR laufen im Fehlerfall in ss_error_fail, das du selbst definierst.",
      lanes: [{ t: "Im Fehlerfall", n: [
        ["api", "SS_ERROR_ASSERT(call)", "call liefert false", ""],
        ["api", "ss_error_fail()", "deine Implementierung", "springt"],
        ["hw", "Error-LED", "toggelt alle 1000 ms"]
      ]},
      { t: "Stack-Überlauf", n: [
        ["obj", "FreeRTOS Stack-Check", ""],
        ["api", "vApplicationStackOverflowHook", "deine Implementierung", "ruft"],
        ["hw", "Error-LED", "toggelt alle 500 ms"]
      ]}]
    },
    eth_udp: {
      sum: "UDP über einen W5500-Ethernet-Chip. Der Task sendet und empfängt Payload-Strukturen pro Port.",
      lanes: [
        { t: "Einmalig", n: [
          ["api", "ss_eth_init(ip, mask, mac, gw)", "", ""],
          ["api", "ss_eth_socket_udp_add(6301, &payload)", "", "danach"],
          ["api", "ss_eth_get(6301, &intf)", "Handle im Task", "danach"]
        ]},
        { t: "Senden und Empfangen", n: [
          ["task", "dein Task", "payload.buffer, buffer_len"],
          ["api", "ss_eth_send(intf, &payload)", "", "ruft"],
          ["hw", "W5500 (per SPI)", "UDP-Socket Port 6301"],
          ["hw", "Ethernet", "192.168.10.122"]
        ]},
        { t: "Empfangen", n: [
          ["hw", "Ethernet", ""],
          ["hw", "W5500", ""],
          ["api", "ss_eth_read(intf, &payload_ptr)", "true, wenn Daten da", "holt ab"],
          ["task", "dein Task", "received_len"]
        ]}
      ]
    },
    pid: {
      sum: "Reine Software: ein PID-Regler mit Anti-Windup. Keine Hardware, kein Interrupt.",
      lanes: [{ t: "Regeln (alle 500 ms)", n: [
        ["task", "dein Task", "Sollwert, Messwert"],
        ["api", "ss_pid_update(&pid, sp, meas, &out)", "kp, ki, kd, tau", "ruft"],
        ["obj", "struct SS_PID", "Integrator (begrenzt), letzter Fehler", "rechnet mit"],
        ["task", "dein Task", "output 0..1"]
      ]}]
    },
    fsm: {
      sum: "Zustandsautomaten über Events: Jeder Task besitzt eine eigene Event-Queue. Ein Haupt-Task verteilt die Events.",
      lanes: [
        { t: "Einmalig", n: [
          ["task", "a_task / b_task", ""],
          ["api", "ss_fsm_event_add(EVENT_START_A)", "Event-Code → Task-Name", "registriert"],
          ["obj", "Event-Queue + Prio-Queue", "je 20 Events, pro Task", "legt an"]
        ]},
        { t: "Event-Fluss", n: [
          ["task", "a_task", "Zustand ON/OFF"],
          ["api", "ss_fsm_event_send_core(e)", "", "meldet"],
          ["obj", "Core-Queue", "", "schreibt"],
          ["task", "main_task", "ss_fsm_event_receive_core()", "liest"],
          ["api", "ss_fsm_event_send(EVENT_START_B)", "sucht Task zum Event-Code", "entscheidet"],
          ["obj", "Queue von b_task", "", "schreibt"],
          ["task", "b_task", "ss_fsm_event_receive(NULL)", "liest"]
        ]}
      ],
      note: "ss_fsm_event_receive liest zuerst die Prio-Queue und erst danach die normale Queue, und wartet nie: gibt es kein Event, kommt -1 zurück."
    }
  };
  var TAG = { hw: "Hardware", isr: "ISR", obj: "RTOS", api: "bsp", task: "Task" };
  var ORDER = ["can_rx", "can_tx", "can_tod", "adc", "pwm", "input", "output", "iob", "fm", "spi", "uart", "printf", "error", "eth_udp", "pid", "fsm"];
  var exPick = document.getElementById("exPick");
  var exView = document.getElementById("exView");

  function mk(tag, cls, text) {
    var e = document.createElement(tag);
    if (cls) e.className = cls;
    if (text) e.textContent = text;
    return e;
  }

  function showEx(name) {
    var d = EX[name];
    exPick.querySelectorAll("button").forEach(function (b) { b.setAttribute("aria-selected", b.dataset.k === name); });
    exView.textContent = "";
    exView.appendChild(mk("p", "ex-sum", d.sum));
    var lanes = mk("div", "ex-lanes");
    exView.appendChild(lanes);
    d.lanes.forEach(function (lane) {
      var wrap = mk("div", "lane");
      wrap.appendChild(mk("div", "lane-t", lane.t));
      var row = mk("div", "lane-row");
      lane.n.forEach(function (n, i) {
        if (i) {
          var ar = mk("div", "arr");
          ar.appendChild(mk("span", "", n[3] || ""));
          ar.appendChild(mk("b", "", "→"));
          row.appendChild(ar);
        }
        var box = mk("div", "nd n-" + n[0]);
        box.appendChild(mk("span", "tag", TAG[n[0]]));
        box.appendChild(mk("code", "", n[1]));
        if (n[2]) box.appendChild(mk("small", "", n[2]));
        row.appendChild(box);
      });
      wrap.appendChild(row);
      lanes.appendChild(wrap);
    });
    if (d.note) exView.appendChild(mk("p", "ex-note", d.note));
  }

  ORDER.forEach(function (k) {
    var b = mk("button", "", k);
    b.type = "button"; b.dataset.k = k; b.setAttribute("role", "tab"); b.setAttribute("aria-selected", "false");
    b.addEventListener("click", function () { showEx(k); });
    exPick.appendChild(b);
  });
  showEx("can_rx");

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
