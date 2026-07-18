(() => {
  const PARAMS = {
    inputGain: { min: -24, max: 24, unit: "dB", decimals: 1 },
    noiseGateThreshold: { min: -100, max: 0, unit: "dB", decimals: 1 },
    outputLevel: { min: -40, max: 12, unit: "dB", decimals: 1 },
    bassGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    midGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    trebleGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
  };

  const TOGGLE_ON = "assets/signal-path/toggle-on.png";
  const TOGGLE_OFF = "assets/signal-path/toggle-off.png";

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
    const pct = valueToPct(id, value);
    document.querySelectorAll(`[data-param="${id}"] .knob`).forEach((el) => {
      el.style.setProperty("--pct", String(pct));
    });
    document.querySelectorAll(`#val-${id}, #val-${id}-main`).forEach((val) => {
      val.textContent = formatValue(id, value);
    });
  }

  function setToggle(id, value) {
    const on = !!value;
    document.querySelectorAll(`#${id}, #${id}Card`).forEach((btn) => {
      if (!btn) return;
      btn.setAttribute("aria-pressed", on ? "true" : "false");
      const img = btn.querySelector("[data-role='toggle-img']");
      if (img) img.src = on ? TOGGLE_ON : TOGGLE_OFF;
    });
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
    const n = Math.max(0, Math.min(1, (db + 60) / 60));
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

  function escapeHtml(text) {
    return String(text)
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;");
  }

  function renderSignalPath() {
    const root = $("signalPath");
    const nodes = [
      {
        title: "INPUT",
        detail: state.parameters.noiseGateEnabled
          ? `Gate ${Number(state.parameters.noiseGateThreshold).toFixed(1)} dB`
          : "Gate off",
        on: true,
        icon: "assets/pedals/pedal-icon.png",
        cls: "",
      },
      {
        title: "AMP",
        detail: state.namLoaded ? state.namName : "No model",
        on: state.namLoaded,
        icon: "assets/amp-head/amp-icon.png",
        cls: "amp",
      },
      {
        title: "EQ",
        detail: state.parameters.eqEnabled ? "Tone active" : "Bypassed",
        on: state.parameters.eqEnabled,
        icon: "assets/signal-path/node-background-tiles.png",
        cls: "",
      },
      {
        title: "CAB",
        detail: state.irLoaded ? state.irName : "No IR",
        on: state.parameters.cabEnabled && state.irLoaded,
        icon: "assets/cabinet/cab-icon.png",
        cls: "",
      },
    ];

    root.innerHTML = nodes
      .map(
        (node) => `
      <div class="path-node ${node.cls} ${node.on ? "on" : ""}">
        <img class="path-icon" src="${node.icon}" alt="" />
        <div>
          <div class="path-title">${node.title}</div>
          <div class="path-detail">${escapeHtml(node.detail)}</div>
        </div>
      </div>`
      )
      .join("");
  }

  function applyState(next) {
    Object.assign(state, next);
    if (next.parameters) state.parameters = { ...state.parameters, ...next.parameters };

    Object.keys(PARAMS).forEach((id) => {
      if (state.parameters[id] !== undefined) setKnob(id, state.parameters[id]);
    });

    setToggle("noiseGateEnabled", state.parameters.noiseGateEnabled);
    setToggle("eqEnabled", state.parameters.eqEnabled);
    setToggle("cabEnabled", state.parameters.cabEnabled);

    $("namName").textContent = state.namLoaded ? state.namName : "None loaded";
    $("irName").textContent = state.irLoaded ? state.irName : "None loaded";
    $("namStatus").textContent = state.namLoaded ? state.namName : "No model";
    $("namStatus").className = `status ${state.namLoaded ? "ok" : "warn"}`;
    $("footerStatus").textContent = state.status || "";

    const cabLed = $("cabLed");
    if (cabLed) cabLed.classList.toggle("on", !!state.parameters.cabEnabled && !!state.irLoaded);

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
      if (!knob) return;
      let dragging = false;
      let startY = 0;
      let startValue = 0;

      const onMove = (clientY) => {
        if (!dragging) return;
        const delta = (startY - clientY) / 1.55;
        const next = pctToValue(id, valueToPct(id, startValue) + delta);
        state.parameters[id] = next;
        setKnob(id, next);
        send({ type: "setParameter", id, value: next });
      };

      knob.addEventListener("pointerdown", (event) => {
        dragging = true;
        startY = event.clientY;
        startValue = state.parameters[id] ?? 0;
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

  function bindToggle(buttonId, paramId) {
    const el = $(buttonId);
    if (!el) return;
    el.addEventListener("click", () => {
      const next = !(state.parameters[paramId]);
      state.parameters[paramId] = next;
      setToggle(paramId === "noiseGateEnabled" ? "noiseGateEnabled" : paramId, next);
      if (paramId === "noiseGateEnabled") setToggle("noiseGateEnabled", next);
      send({ type: "setParameter", id: paramId, value: next ? 1 : 0 });
      renderSignalPath();
      if (paramId === "cabEnabled") {
        const cabLed = $("cabLed");
        if (cabLed) cabLed.classList.toggle("on", next && state.irLoaded);
      }
    });
  }

  function bindControls() {
    bindToggle("noiseGateEnabled", "noiseGateEnabled");
    bindToggle("noiseGateEnabledCard", "noiseGateEnabled");
    bindToggle("eqEnabled", "eqEnabled");
    bindToggle("cabEnabled", "cabEnabled");

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
