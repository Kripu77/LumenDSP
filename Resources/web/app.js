(() => {
  const PARAMS = {
    inputGain: { min: -24, max: 24, unit: "dB", decimals: 1 },
    noiseGateThreshold: { min: -100, max: 0, unit: "dB", decimals: 1 },
    outputLevel: { min: -40, max: 12, unit: "dB", decimals: 1 },
    bassGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    midGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    trebleGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
  };

  const state = {
    parameters: {
      inputGain: 0,
      noiseGateThreshold: -80,
      noiseGateEnabled: true,
      outputLevel: 0,
      bassGain: 0,
      midGain: 0,
      trebleGain: 0,
      eqEnabled: true,
      cabEnabled: true,
    },
    presets: [],
    currentPreset: "",
    namName: "",
    irName: "",
    namLoaded: false,
    irLoaded: false,
    status: "Connecting…",
  };

  const $ = (id) => document.getElementById(id);

  function send(message) {
    const payload = typeof message === "string" ? message : JSON.stringify(message);
    try {
      if (window.__JUCE__?.backend?.emitEvent) {
        window.__JUCE__.backend.emitEvent("lumenMessage", payload);
        return;
      }
      if (window.lumenPostMessage) {
        window.lumenPostMessage(payload);
        return;
      }
      console.log("send", message);
    } catch (err) {
      console.error(err);
    }
  }

  function formatValue(id, value) {
    const meta = PARAMS[id];
    if (!meta) return String(value);
    return `${Number(value).toFixed(meta.decimals)} ${meta.unit}`;
  }

  function valueToPct(id, value) {
    const meta = PARAMS[id];
    if (!meta) return 0;
    return ((value - meta.min) / (meta.max - meta.min)) * 100;
  }

  function pctToValue(id, pct) {
    const meta = PARAMS[id];
    if (!meta) return 0;
    const raw = meta.min + (pct / 100) * (meta.max - meta.min);
    const stepped = Math.round(raw * 10) / 10;
    return Math.min(meta.max, Math.max(meta.min, stepped));
  }

  function setKnob(id, value) {
    const el = $(`knob-${id}`);
    const val = $(`val-${id}`);
    if (!el) return;
    const pct = valueToPct(id, value);
    el.style.setProperty("--pct", String(pct));
    if (val) val.textContent = formatValue(id, value);
  }

  function setBool(id, value) {
    const el = $(id);
    if (el) el.checked = !!value;
  }

  function buildMeters() {
    for (const id of ["input-meter", "output-meter"]) {
      const root = $(id);
      root.innerHTML = "";
      for (let i = 0; i < 20; i += 1) {
        const seg = document.createElement("div");
        seg.className = "meter-seg";
        root.appendChild(seg);
      }
    }
  }

  function paintMeter(id, db) {
    const root = $(id);
    if (!root) return;
    const segs = [...root.children];
    const floor = -60;
    const ceil = 0;
    const n = Math.max(0, Math.min(1, (db - floor) / (ceil - floor)));
    const lit = Math.round(n * segs.length);
    segs.forEach((seg, index) => {
      seg.classList.remove("on", "warn", "clip");
      if (index < lit) {
        const level = (index + 1) / segs.length;
        if (level > 0.92) seg.classList.add("clip");
        else if (level > 0.78) seg.classList.add("warn");
        else seg.classList.add("on");
      }
    });
  }

  function renderSignalPath() {
    const root = $("signalPath");
    const nodes = [
      {
        key: "input",
        title: "INPUT",
        detail: state.parameters.noiseGateEnabled
          ? `Gate ${state.parameters.noiseGateThreshold.toFixed(1)} dB`
          : "Gate off",
        on: true,
        cls: "",
      },
      {
        key: "amp",
        title: "AMP",
        detail: state.namLoaded ? state.namName : "No model",
        on: state.namLoaded,
        cls: "amp",
      },
      {
        key: "eq",
        title: "EQ",
        detail: state.parameters.eqEnabled ? "Tone active" : "Bypassed",
        on: state.parameters.eqEnabled,
        cls: "",
      },
      {
        key: "cab",
        title: "CAB",
        detail: state.irLoaded ? state.irName : "No IR",
        on: state.parameters.cabEnabled && state.irLoaded,
        cls: "",
      },
    ];

    root.innerHTML = nodes
      .map(
        (node) => `
      <div class="path-node ${node.cls} ${node.on ? "on" : ""}">
        <div class="path-title"><span class="dot"></span>${node.title}</div>
        <div class="path-detail">${escapeHtml(node.detail)}</div>
      </div>`
      )
      .join("");
  }

  function escapeHtml(text) {
    return String(text)
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;");
  }

  function applyState(next) {
    Object.assign(state, next);
    if (next.parameters) state.parameters = { ...state.parameters, ...next.parameters };

    Object.keys(PARAMS).forEach((id) => {
      if (state.parameters[id] !== undefined) setKnob(id, state.parameters[id]);
    });
    setBool("noiseGateEnabled", state.parameters.noiseGateEnabled);
    setBool("eqEnabled", state.parameters.eqEnabled);
    setBool("cabEnabled", state.parameters.cabEnabled);

    $("namName").textContent = state.namLoaded ? state.namName : "None loaded";
    $("irName").textContent = state.irLoaded ? state.irName : "None loaded";
    $("namStatus").textContent = state.namLoaded ? state.namName : "No model";
    $("namStatus").className = `status ${state.namLoaded ? "ok" : "warn"}`;
    $("footerStatus").textContent = state.status || "";

    const select = $("presetSelect");
    const previous = select.value;
    select.innerHTML = "";
    (state.presets || []).forEach((name) => {
      const opt = document.createElement("option");
      opt.value = name;
      opt.textContent = name;
      select.appendChild(opt);
    });
    if (state.currentPreset) select.value = state.currentPreset;
    else if (previous) select.value = previous;

    renderSignalPath();
  }

  function bindKnobs() {
    document.querySelectorAll(".knob-wrap[data-param]").forEach((wrap) => {
      const id = wrap.dataset.param;
      const knob = wrap.querySelector(".knob");
      let dragging = false;
      let startY = 0;
      let startValue = 0;

      const onMove = (clientY) => {
        if (!dragging) return;
        const delta = (startY - clientY) / 1.6;
        const next = pctToValue(id, valueToPct(id, startValue) + delta);
        state.parameters[id] = next;
        setKnob(id, next);
        send({ type: "setParameter", id, value: next });
      };

      knob.addEventListener("pointerdown", (event) => {
        dragging = true;
        startY = event.clientY;
        startValue = state.parameters[id];
        knob.setPointerCapture(event.pointerId);
      });
      knob.addEventListener("pointermove", (event) => onMove(event.clientY));
      knob.addEventListener("pointerup", () => {
        dragging = false;
      });
      knob.addEventListener("pointercancel", () => {
        dragging = false;
      });
    });
  }

  function bindControls() {
    ["noiseGateEnabled", "eqEnabled", "cabEnabled"].forEach((id) => {
      $(id).addEventListener("change", (event) => {
        const value = !!event.target.checked;
        state.parameters[id] = value;
        send({ type: "setParameter", id, value: value ? 1 : 0 });
        renderSignalPath();
      });
    });

    $("presetSelect").addEventListener("change", (event) => {
      send({ type: "loadPreset", name: event.target.value });
    });
    $("btnLoad").addEventListener("click", () => {
      send({ type: "loadPreset", name: $("presetSelect").value });
    });
    $("btnSave").addEventListener("click", () => {
      const name = $("presetName").value.trim() || $("presetSelect").value;
      if (name) send({ type: "savePreset", name });
    });
    $("btnBrowseNam").addEventListener("click", () => send({ type: "browseNam" }));
    $("btnBrowseIr").addEventListener("click", () => send({ type: "browseIr" }));
    $("btnAudio").addEventListener("click", () => send({ type: "openAudioSettings" }));
  }

  window.__lumenReceive = function receive(message) {
    let data = message;
    if (typeof message === "string") {
      try {
        data = JSON.parse(message);
      } catch {
        return;
      }
    }
    if (!data || !data.type) return;

    if (data.type === "state") {
      applyState(data);
    } else if (data.type === "meters") {
      paintMeter("input-meter", data.inputDb ?? -60);
      paintMeter("output-meter", data.outputDb ?? -60);
    } else if (data.type === "status") {
      $("footerStatus").textContent = data.message || "";
    }
  };

  buildMeters();
  bindKnobs();
  bindControls();
  applyState(state);
  send({ type: "ready" });
})();
