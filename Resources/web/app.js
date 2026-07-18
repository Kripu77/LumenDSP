(() => {
  const PARAMS = {
    inputGain: { min: -24, max: 24, unit: "dB", decimals: 1 },
    noiseGateThreshold: { min: -100, max: 0, unit: "dB", decimals: 1 },
    outputLevel: { min: -40, max: 12, unit: "dB", decimals: 1 },
    bassGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    midGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    trebleGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    metronomeBpm: { min: 40, max: 240, unit: "BPM", decimals: 0 },
    metronomeVolume: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
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
      metronomeEnabled: false,
      metronomeBpm: 120,
      metronomeVolume: 0.35,
    },
    presets: [],
    currentPreset: "",
    namName: "",
    irName: "",
    namLoaded: false,
    irLoaded: false,
    status: "Connecting…",
    openPopover: null,
    tunerMode: "cents",
    liveTuner: true,
  };

  const tapTimes = [];

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
    if (meta.asPercent)
      return `${Math.round(Number(value) * 100)} ${meta.unit}`;
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
        icon: "assets/signal-path/icon-eq.png",
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
    setToggle("metronomeEnabled", state.parameters.metronomeEnabled);

    $("namName").textContent = state.namLoaded ? state.namName : "None loaded";
    $("irName").textContent = state.irLoaded ? state.irName : "None loaded";
    $("namStatus").textContent = state.namLoaded ? state.namName : "No model";
    $("namStatus").className = `status ${state.namLoaded ? "ok" : "warn"}`;
    $("footerStatus").textContent = state.status || "";

    const dockBpm = $("dockBpm");
    if (dockBpm && state.parameters.metronomeBpm !== undefined)
      dockBpm.textContent = `${Number(state.parameters.metronomeBpm).toFixed(1)} BPM`;
    const big = $("metroBpmBig");
    if (big && state.parameters.metronomeBpm !== undefined)
      big.textContent = Number(state.parameters.metronomeBpm).toFixed(1);
    const play = $("metroPlayIcon");
    if (play) play.textContent = state.parameters.metronomeEnabled ? "■" : "▶";
    const dockPlay = $("dockMetroPlay");
    if (dockPlay) {
      dockPlay.textContent = state.parameters.metronomeEnabled ? "■" : "▶";
      dockPlay.classList.toggle("on", !!state.parameters.metronomeEnabled);
    }

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

  function setTempo(bpm) {
    const meta = PARAMS.metronomeBpm;
    const next = Math.min(meta.max, Math.max(meta.min, Math.round(Number(bpm) * 10) / 10));
    state.parameters.metronomeBpm = next;
    const dock = $("dockBpm");
    const big = $("metroBpmBig");
    if (dock) dock.textContent = `${next.toFixed(1)} BPM`;
    if (big) big.textContent = next.toFixed(1);
    send({ type: "setParameter", id: "metronomeBpm", value: next });
  }

  function nudgeTempo(delta) {
    setTempo((state.parameters.metronomeBpm ?? 120) + delta);
  }

  function handleTapTempo() {
    const now = performance.now();
    if (tapTimes.length && now - tapTimes[tapTimes.length - 1] > 2000)
      tapTimes.length = 0;
    tapTimes.push(now);
    if (tapTimes.length > 6) tapTimes.shift();
    if (tapTimes.length < 2) return;

    let total = 0;
    for (let i = 1; i < tapTimes.length; i += 1)
      total += tapTimes[i] - tapTimes[i - 1];
    setTempo(60000 / (total / (tapTimes.length - 1)));
  }

  function closePopovers() {
    state.openPopover = null;
    const backdrop = $("popoverBackdrop");
    const tuner = $("tunerPopover");
    const metro = $("metroPopover");
    if (backdrop) backdrop.hidden = true;
    if (tuner) tuner.hidden = true;
    if (metro) metro.hidden = true;
    $("btnTuner")?.setAttribute("aria-pressed", "false");
    $("btnMetro")?.setAttribute("aria-pressed", "false");
  }

  function openPopover(name) {
    if (state.openPopover === name) {
      closePopovers();
      return;
    }
    closePopovers();
    state.openPopover = name;
    const backdrop = $("popoverBackdrop");
    if (backdrop) backdrop.hidden = false;
    if (name === "tuner") {
      $("tunerPopover").hidden = false;
      $("btnTuner")?.setAttribute("aria-pressed", "true");
    }
    if (name === "metro") {
      $("metroPopover").hidden = false;
      $("btnMetro")?.setAttribute("aria-pressed", "true");
    }
  }

  function paintBeats(beatInBar, beatsPerBar, running) {
    const root = $("metroBeats");
    if (!root) return;
    const dots = [...root.querySelectorAll(".beat-dot")];
    dots.forEach((dot, index) => {
      if (index >= beatsPerBar) {
        dot.style.display = "none";
        return;
      }
      dot.style.display = "";
      const active = running && index === beatInBar;
      dot.classList.toggle("on", active);
    });
  }

  function paintPractice(data) {
    const live = state.liveTuner;
    const locked = live && !!data.locked;
    const cents = locked ? Math.max(-50, Math.min(50, Number(data.cents) || 0)) : 0;
    const hz = locked ? Number(data.frequencyHz) || 0 : 0;

    const note = $("tunerNote");
    const centsEl = $("tunerCents");
    const hzEl = $("tunerHz");
    const orb = $("tunerOrb");

    if (note) note.textContent = locked ? (data.note || "--") : "--";
    if (centsEl) {
      centsEl.textContent = locked
        ? `${cents > 0 ? "+" : ""}${Math.round(cents)} ¢`
        : "—";
      centsEl.style.display = state.tunerMode === "cents" ? "" : "none";
    }
    if (hzEl) {
      hzEl.textContent = locked ? `${hz.toFixed(1)} Hz` : "— Hz";
      hzEl.style.display = state.tunerMode === "hz" ? "" : "none";
    }
    if (orb) {
      const shift = (cents / 50) * 42;
      orb.style.transform = `translateX(${shift}%)`;
      orb.classList.toggle("in-tune", locked && Math.abs(cents) < 5);
    }

    if (data.metronomeEnabled !== undefined) {
      state.parameters.metronomeEnabled = !!data.metronomeEnabled;
      setToggle("metronomeEnabled", data.metronomeEnabled);
      const dockPlay = $("dockMetroPlay");
      if (dockPlay) {
        dockPlay.textContent = data.metronomeEnabled ? "■" : "▶";
        dockPlay.classList.toggle("on", !!data.metronomeEnabled);
      }
      const icon = $("metroPlayIcon");
      if (icon) icon.textContent = data.metronomeEnabled ? "■" : "▶";
      $("btnMetro")?.classList.toggle("running", !!data.metronomeEnabled);
    }

    if (data.metronomeBpm !== undefined) {
      state.parameters.metronomeBpm = data.metronomeBpm;
      const dock = $("dockBpm");
      const big = $("metroBpmBig");
      if (dock) dock.textContent = `${Number(data.metronomeBpm).toFixed(1)} BPM`;
      if (big) big.textContent = Number(data.metronomeBpm).toFixed(1);
    }

    if (data.metronomeVolume !== undefined) {
      state.parameters.metronomeVolume = data.metronomeVolume;
      setKnob("metronomeVolume", data.metronomeVolume);
    }

    paintBeats(
      Number(data.beatInBar) || 0,
      Number(data.beatsPerBar) || 4,
      !!data.metronomeEnabled
    );
  }

  function bindControls() {
    bindToggle("noiseGateEnabled", "noiseGateEnabled");
    bindToggle("noiseGateEnabledCard", "noiseGateEnabled");
    bindToggle("eqEnabled", "eqEnabled");
    bindToggle("cabEnabled", "cabEnabled");
    bindToggle("metronomeEnabled", "metronomeEnabled");

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
    $("btnAudio")?.addEventListener("click", () => send({ type: "openAudioSettings" }));

    $("btnTuner")?.addEventListener("click", () => openPopover("tuner"));
    $("btnMetro")?.addEventListener("click", () => openPopover("metro"));
    $("btnTunerClose")?.addEventListener("click", closePopovers);
    $("btnMetroClose")?.addEventListener("click", closePopovers);
    $("popoverBackdrop")?.addEventListener("click", closePopovers);

    $("btnTap")?.addEventListener("click", handleTapTempo);
    $("btnDockTap")?.addEventListener("click", handleTapTempo);
    $("btnTempoMinus")?.addEventListener("click", () => nudgeTempo(-1));
    $("btnTempoPlus")?.addEventListener("click", () => nudgeTempo(1));

    document.querySelectorAll("[data-tuner-mode]").forEach((btn) => {
      btn.addEventListener("click", () => {
        state.tunerMode = btn.getAttribute("data-tuner-mode") || "cents";
        document.querySelectorAll("[data-tuner-mode]").forEach((el) => {
          el.classList.toggle("active", el === btn);
        });
        const centsEl = $("tunerCents");
        const hzEl = $("tunerHz");
        if (centsEl) centsEl.style.display = state.tunerMode === "cents" ? "" : "none";
        if (hzEl) hzEl.style.display = state.tunerMode === "hz" ? "" : "none";
      });
    });

    $("liveTuner")?.addEventListener("change", (event) => {
      state.liveTuner = !!event.target.checked;
    });
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
    } else if (data.type === "practice") {
      paintPractice(data);
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
