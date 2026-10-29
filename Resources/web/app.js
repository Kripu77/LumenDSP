(() => {
  const PARAMS = {
    inputGain: { min: -24, max: 24, unit: "dB", decimals: 1 },
    noiseGateThreshold: { min: -100, max: 0, unit: "dB", decimals: 1 },
    outputLevel: { min: -40, max: 12, unit: "dB", decimals: 1 },
    bassGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    midGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    trebleGain: { min: -12, max: 12, unit: "dB", decimals: 1 },
    ampMix: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    cabMix: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    calibrationGain: { min: -24, max: 24, unit: "dB", decimals: 1, local: true },
    metronomeBpm: { min: 40, max: 240, unit: "BPM", decimals: 0 },
    metronomeVolume: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    compressorThreshold: { min: -40, max: 0, unit: "dB", decimals: 1 },
    compressorRatio: { min: 1, max: 20, unit: ":1", decimals: 1 },
    compressorMix: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    driveAmount: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    driveTone: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    driveLevel: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    delayTime: { min: 1, max: 1000, unit: "ms", decimals: 0 },
    delayFeedback: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    delayMix: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    delayTone: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    delayModulation: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    delaySaturation: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    doublerSpread: { min: 3, max: 20, unit: "ms", decimals: 1 },
    reverbDecay: { min: 0.3, max: 20, unit: "s", decimals: 1 },
    reverbLowCut: { min: 20, max: 700, unit: "Hz", decimals: 0 },
    reverbHighCut: { min: 1000, max: 12000, unit: "Hz", decimals: 0 },
    reverbWidth: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    reverbMix: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
  };

  const SECTIONS = ["pedals", "amp", "cab", "effects"];
  const DELAY_DIVISIONS = ["1/4", "1/8", "1/8d", "1/16", "1/4d", "1/2"];
  const CHAIN_ICONS = {
    comp: "M3 12h6M6.5 9.5 9 12l-2.5 2.5M21 12h-6M17.5 9.5 15 12l2.5 2.5M12 5v14",
    drive: "M12 3c.8 3.2 5 4.8 5 9.6A5 5 0 0 1 7 12.6c0-2.3 1.2-3.6 2.4-4.6.2 2 1.2 3.2 2.4 3.4-.6-3 .2-5.6.2-8.4Z",
    amp: "M3 6.5h18v11H3zM3 11h18M7 14.3v.01M10.5 14.3v.01M14 14.3v.01M18 14.3v.01",
    cab: "M4 3h16v18H4zM12 7.5a4.5 4.5 0 1 0 0 9 4.5 4.5 0 0 0 0-9ZM12 11.2v1.6",
    delay: "M4 12h.01M8 8.5a5 5 0 0 1 0 7M12.5 6a8.5 8.5 0 0 1 0 12M17 3.5a12 12 0 0 1 0 17",
    reverb: "M3 8c3-2.5 6 2.5 9 0s6 2.5 9 0M3 12c3-2.5 6 2.5 9 0s6 2.5 9 0M3 16c3-2.5 6 2.5 9 0s6 2.5 9 0",
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
      ampMix: 1,
      cabMix: 1,
      calibrationGain: 0,
      metronomeEnabled: false,
      metronomeBpm: 120,
      metronomeVolume: 0.35,
      compressorEnabled: false,
      compressorThreshold: -18,
      compressorRatio: 4,
      compressorMix: 1,
      driveEnabled: false,
      driveMode: 0,
      driveAmount: 0.35,
      driveTone: 0.5,
      driveLevel: 0.7,
      delayEnabled: false,
      delaySync: false,
      delayDivision: 0,
      delayTime: 380,
      delayFeedback: 0.25,
      delayMix: 0.2,
      delayTone: 0.5,
      delayModulation: 0.3,
      delaySaturation: 0.2,
      delayPingPong: false,
      doublerEnabled: false,
      doublerSpread: 6,
      reverbEnabled: false,
      reverbCharacter: 0,
      reverbDecay: 4,
      reverbLowCut: 180,
      reverbHighCut: 8000,
      reverbWidth: 1,
      reverbMix: 0.18,
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
    libraryTab: "models",
    librarySearch: "",
    libraryCursor: 0,
    library: { models: [], irs: [] },
    presetsMeta: [],
    presetCategories: [],
    presetsSearch: "",
    presetsCategory: "",
    presetsCursor: 0,
    demo: { playing: false, activeClipId: "", activeClipName: "", clips: [] },
    settings: {
      activeCalibrationId: "",
      activeCalibrationName: "",
      activeCalibrationGainDb: 0,
      calibrationProfiles: [],
    },
    cpuPercent: 0,
    hotInput: false,
    hotOutput: false,
  };

  const DEFAULTS = { ...state.parameters };
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
      return `${Math.round(Number(value) * 100)}${meta.unit === "%" ? "%" : ` ${meta.unit}`}`;
    if (meta.unit === ":1")
      return `${Number(value).toFixed(meta.decimals)}${meta.unit}`;
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
    const clamped = Math.min(100, Math.max(0, pct));
    const meta = PARAMS[id];
    const origin = meta && meta.min < 0 && meta.max > 0 ? valueToPct(id, 0) : 0;
    document.querySelectorAll(`[data-param="${id}"] .knob`).forEach((el) => {
      el.style.setProperty("--frame", String(Math.round(clamped * 0.64)));
      el.style.setProperty("--angle", `${clamped * 2.7 - 135}deg`);
      el.style.setProperty("--a0", `${Math.min(clamped, origin) * 2.7}deg`);
      el.style.setProperty("--a1", `${Math.max(clamped, origin) * 2.7}deg`);
      el.setAttribute("aria-valuetext", formatValue(id, value));
    });
    const val = $(`val-${id}`);
    if (val) val.textContent = formatValue(id, value);
  }

  function setToggle(id, value) {
    $(id)?.setAttribute("aria-pressed", value ? "true" : "false");
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

  function showSection(name) {
    const index = SECTIONS.indexOf(name);
    if (index < 0) return;
    state.section = name;
    document.body.dataset.section = name;
    document.querySelectorAll(".scene").forEach((scene) => {
      const sceneIndex = SECTIONS.indexOf(scene.dataset.scene);
      scene.dataset.pos = sceneIndex < index ? "before" : sceneIndex > index ? "after" : "active";
      scene.inert = sceneIndex !== index;
    });
    document.querySelectorAll("[data-section-tab]").forEach((tab) => {
      if (tab.dataset.sectionTab === name) tab.setAttribute("aria-current", "page");
      else tab.removeAttribute("aria-current");
    });
    renderSignalPath();
  }

  function renderSignalPath() {
    const root = $("signalPath");
    if (!root) return;
    const p = state.parameters;
    const nodes = [
      { title: "squeeze", icon: "comp", section: "pedals", on: p.compressorEnabled, detail: `${Number(p.compressorRatio).toFixed(1)}:1` },
      { title: "ember", icon: "drive", section: "pedals", on: p.driveEnabled, detail: ["soft", "hard", "tube", "boost"][Math.round(p.driveMode) || 0] },
      { title: "amp", icon: "amp", section: "amp", on: state.namLoaded, detail: state.namLoaded ? state.namName.replace(/\.nam$/i, "") : "no model" },
      { title: "cab", icon: "cab", section: "cab", on: p.cabEnabled && state.irLoaded, detail: state.irLoaded ? state.irName.replace(/\.wav$/i, "") : "no IR" },
      { title: "echo", icon: "delay", section: "effects", on: p.delayEnabled, detail: delayReadout() },
      { title: "haze", icon: "reverb", section: "effects", on: p.reverbEnabled, detail: `${["room", "hall", "plate", "ambient"][Math.round(p.reverbCharacter) || 0]} ${Number(p.reverbDecay).toFixed(1)} s` },
    ];

    root.innerHTML = `<span class="chain-jack">in</span>${nodes
      .map(
        (node) => `
      <button type="button" class="path-node ${node.on ? "on" : ""} ${node.section === state.section ? "current" : ""}" data-goto="${node.section}" data-icon="${node.icon}">
        <span class="path-tile"><svg viewBox="0 0 24 24" aria-hidden="true"><path d="${CHAIN_ICONS[node.icon]}"/></svg></span>
        <span class="path-text">
          <span class="path-title">${node.title}</span>
          <span class="path-detail">${escapeHtml(node.detail)}</span>
        </span>
      </button>`
      )
      .join("")}<span class="chain-jack">out</span>`;
  }

  function delayReadout() {
    const p = state.parameters;
    const time = p.delaySync ? DELAY_DIVISIONS[Math.round(p.delayDivision) || 0] : `${Math.round(p.delayTime)} ms`;
    return p.delayPingPong ? `${time} ↔` : time;
  }

  function stepPreset(delta) {
    const names = state.presets || [];
    if (!names.length) return;
    const index = names.indexOf(state.currentPreset);
    const next = names[(index + delta + names.length) % names.length];
    send({ type: "loadPreset", name: next });
  }

  function bindStage() {
    $("signalPath")?.addEventListener("click", (event) => {
      const node = event.target.closest("[data-goto]");
      if (node) showSection(node.dataset.goto);
    });
    document.querySelectorAll("[data-section-tab]").forEach((tab) => {
      tab.addEventListener("click", () => showSection(tab.dataset.sectionTab));
    });
    $("btnPresetPrev")?.addEventListener("click", () => stepPreset(-1));
    $("btnPresetNext")?.addEventListener("click", () => stepPreset(1));

  }

  function applyState(next) {
    Object.assign(state, next, { parameters: { ...state.parameters, ...next.parameters } });

    Object.keys(PARAMS).forEach((id) => {
      if (state.parameters[id] !== undefined) setKnob(id, state.parameters[id]);
    });

    setToggle("noiseGateEnabled", state.parameters.noiseGateEnabled);
    setToggle("eqEnabled", state.parameters.eqEnabled);
    setToggle("cabEnabled", state.parameters.cabEnabled);
    setToggle("metronomeEnabled", state.parameters.metronomeEnabled);
    setToggle("compressorEnabled", state.parameters.compressorEnabled);
    setToggle("driveEnabled", state.parameters.driveEnabled);
    setToggle("delayEnabled", state.parameters.delayEnabled);
    setToggle("delaySync", state.parameters.delaySync);
    setToggle("delayPingPong", state.parameters.delayPingPong);
    setToggle("doublerEnabled", state.parameters.doublerEnabled);
    setToggle("reverbEnabled", state.parameters.reverbEnabled);

    syncChoiceChips("driveMode", state.parameters.driveMode);
    syncChoiceChips("delayDivision", state.parameters.delayDivision);
    syncChoiceChips("reverbCharacter", state.parameters.reverbCharacter);
    updateFxModuleClasses();

    $("namName").textContent = state.namLoaded ? state.namName : "None loaded";
    $("irName").textContent = state.irLoaded ? state.irName : "None loaded";
    $("namStatus").textContent = state.namLoaded ? state.namName : "No amp model loaded. Choose one to hear the amp.";
    $("namStatus").classList.toggle("ok", !!state.namLoaded);
    document.body.classList.toggle("nam-loaded", !!state.namLoaded);
    document.body.classList.toggle("cab-off", !state.parameters.cabEnabled);
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

    const currentName = $("presetCurrentName");
    if (currentName)
      currentName.textContent = state.currentPreset ? displayName(state.currentPreset) : "No preset";

    renderSignalPath();
  }

  function clipsSignature(clips) {
    return (clips || []).map((c) => `${c.id}|${c.name}`).join(";");
  }

  function renderDemoUi() {
    const select = $("demoClipSelect");
    if (select) {
      const clips = state.demo.clips || [];
      const nextSig = clipsSignature(clips);
      const prevSig = select.dataset.clipsSig || "";
      const preferred = state.demo.activeClipId || select.value || (clips[0] && clips[0].id) || "";

      // Only rebuild options when the clip list changes — wiping <select>
      // while open made it impossible to pick a clip.
      if (nextSig !== prevSig) {
        select.innerHTML = "";
        clips.forEach((clip) => {
          const opt = document.createElement("option");
          opt.value = clip.id;
          opt.textContent = clip.name;
          select.appendChild(opt);
        });
        select.dataset.clipsSig = nextSig;
      }

      if (preferred && select.value !== preferred) {
        const has = Array.from(select.options).some((o) => o.value === preferred);
        if (has) select.value = preferred;
      }
    }

    const play = $("dockDemoPlay");
    const btn = $("btnDemoPlay");
    if (play) play.textContent = state.demo.playing ? "■" : "▶";
    if (btn) {
      btn.classList.toggle("on", !!state.demo.playing);
      btn.setAttribute("aria-pressed", state.demo.playing ? "true" : "false");
      btn.title = state.demo.playing
        ? "Stop demo DI"
        : "Play demo DI through the chain";
    }
  }

  function renderCalibrationUi() {
    const select = $("calibrationSelect");
    if (select) {
      select.innerHTML = "";
      (state.settings.calibrationProfiles || []).forEach((profile) => {
        const opt = document.createElement("option");
        opt.value = profile.id;
        opt.textContent = `${profile.name} (${Number(profile.gainDb).toFixed(1)} dB)`;
        select.appendChild(opt);
      });
      if (state.settings.activeCalibrationId)
        select.value = state.settings.activeCalibrationId;
    }
    const nameInput = $("calibrationName");
    if (nameInput && document.activeElement !== nameInput)
      nameInput.value = state.settings.activeCalibrationName || "";
  }

  function bindKnobs() {
    document.querySelectorAll(".knob-wrap[data-param]").forEach((wrap) => {
      const id = wrap.dataset.param;
      const knob = wrap.querySelector(".knob");
      if (!knob) return;
      let dragging = false;
      let startY = 0;
      let startValue = 0;
      knob.tabIndex = 0;
      knob.setAttribute("role", "slider");
      knob.setAttribute("aria-label", wrap.querySelector(".knob-label")?.textContent || id);
      knob.addEventListener("dblclick", () => {
        if (DEFAULTS[id] !== undefined) commit(DEFAULTS[id]);
      });

      const commit = (next) => {
        state.parameters[id] = next;
        setKnob(id, next);
        if (PARAMS[id]?.local || wrap.dataset.local === "true") {
          if (id === "calibrationGain")
            send({ type: "setCalibrationGain", gainDb: next });
          return;
        }
        send({ type: "setParameter", id, value: next });
      };

      const onMove = (clientY) => {
        if (!dragging) return;
        commit(pctToValue(id, valueToPct(id, startValue) + (startY - clientY) / 1.55));
      };

      knob.addEventListener("keydown", (event) => {
        const step = { ArrowUp: 1, ArrowRight: 1, ArrowDown: -1, ArrowLeft: -1 }[event.key];
        if (!step) return;
        event.preventDefault();
        const current = state.parameters[id] ?? 0;
        commit(pctToValue(id, valueToPct(id, current) + step * (event.shiftKey ? 10 : 1)));
      });

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

  function updateFxModuleClasses() {
    document.querySelectorAll(".fx-module").forEach((mod) => {
      const key = mod.getAttribute("data-fx");
      const enabled =
        key === "comp" ? state.parameters.compressorEnabled
        : key === "drive" ? state.parameters.driveEnabled
        : key === "delay" ? state.parameters.delayEnabled
        : key === "reverb" ? state.parameters.reverbEnabled
        : false;
      mod.classList.toggle("on", !!enabled);
      if (key === "delay")
        mod.classList.toggle("sync-on", !!state.parameters.delaySync);
    });
    const lcd = $("delayLcd");
    if (lcd) lcd.textContent = delayReadout();
  }

  function syncChoiceChips(choiceId, value) {
    const group = document.querySelector(`.fx-choice[data-choice="${choiceId}"]`);
    if (!group) return;
    const index = Math.round(Number(value) || 0);
    group.querySelectorAll(".fx-chip").forEach((chip) => {
      chip.classList.toggle("active", Number(chip.dataset.value) === index);
    });
  }

  function setChoiceParam(choiceId, value) {
    const index = Math.round(Number(value) || 0);
    state.parameters[choiceId] = index;
    syncChoiceChips(choiceId, index);
    send({ type: "setParameter", id: choiceId, value: index });
    renderSignalPath();
  }

  function bindChoiceGroups() {
    document.querySelectorAll(".fx-choice[data-choice]").forEach((group) => {
      if (group.dataset.bound === "1") return;
      group.dataset.bound = "1";
      group.addEventListener("click", (event) => {
        const chip = event.target.closest(".fx-chip");
        if (!chip || !group.contains(chip)) return;
        setChoiceParam(group.getAttribute("data-choice"), chip.dataset.value);
      });
    });
  }

  function bindToggle(buttonId, paramId) {
    const el = $(buttonId);
    if (!el) return;
    el.addEventListener("click", () => {
      const next = !(state.parameters[paramId]);
      state.parameters[paramId] = next;
      setToggle(paramId, next);
      send({ type: "setParameter", id: paramId, value: next ? 1 : 0 });
      renderSignalPath();
      updateFxModuleClasses();
      if (paramId === "cabEnabled") {
        $("cabLed")?.classList.toggle("on", next && state.irLoaded);
        document.body.classList.toggle("cab-off", !next);
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
    const library = $("libraryPopover");
    const presets = $("presetsPopover");
    const settings = $("settingsPopover");
    if (backdrop) backdrop.hidden = true;
    if (tuner) tuner.hidden = true;
    if (metro) metro.hidden = true;
    if (library) library.hidden = true;
    if (presets) presets.hidden = true;
    if (settings) settings.hidden = true;
    $("btnTuner")?.setAttribute("aria-pressed", "false");
    $("btnMetro")?.setAttribute("aria-pressed", "false");
    $("btnLibrary")?.setAttribute("aria-pressed", "false");
    $("btnSettings")?.setAttribute("aria-pressed", "false");
    $("btnPresets")?.setAttribute("aria-expanded", "false");
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
    if (name === "library") {
      $("libraryPopover").hidden = false;
      $("btnLibrary")?.setAttribute("aria-pressed", "true");
      send({ type: "refreshLibrary" });
      state.libraryCursor = Math.max(0, filteredLibraryItems().findIndex(isLibraryItemLoaded));
      renderLibraryList();
      $("librarySearch")?.focus();
    }
    if (name === "presets") {
      $("presetsPopover").hidden = false;
      $("btnPresets")?.setAttribute("aria-expanded", "true");
      send({ type: "refreshPresets" });
      state.presetsCursor = Math.max(0, filteredPresets().findIndex((item) => item.name === state.currentPreset));
      renderPresetsList();
      $("presetsSearch")?.focus();
    }
    if (name === "settings") {
      $("settingsPopover").hidden = false;
      $("btnSettings")?.setAttribute("aria-pressed", "true");
      send({ type: "refreshSettings" });
      renderCalibrationUi();
    }
  }

  const CATEGORY_COLORS = { clean: "var(--sky)", crunch: "var(--amber)", lead: "var(--coral)", ambient: "var(--violet)" };
  // "04 Full Rig Clean" sorts by its number but reads as "Full Rig Clean"
  const displayName = (name) => String(name).replace(/^\d+\s+/, "");
  const categoryColor = (category) => CATEGORY_COLORS[String(category).toLowerCase()] || "var(--faint)";
  const ICON_STAR = '<svg viewBox="0 0 24 24"><path d="m12 3.5 2.6 5.3 5.9.9-4.3 4.1 1 5.8-5.2-2.7-5.2 2.7 1-5.8-4.3-4.1 5.9-.9z"/></svg>';
  const ICON_CHECK = '<svg viewBox="0 0 24 24"><path d="m5 12.5 4.5 4.5L19 7.5"/></svg>';
  const ICON_EXPORT = '<svg viewBox="0 0 24 24"><path d="M12 15V4m0 0L8 8m4-4 4 4M5 14v5h14v-5"/></svg>';
  const ICON_DELETE = '<svg viewBox="0 0 24 24"><path d="M5 7h14M10 7V5h4v2m-7 0 1 12h8l1-12"/></svg>';

  const sideItemsHtml = (entries, activeKey, attr) => entries
    .map((e) => `
        <button type="button" class="side-item ${activeKey === e.key ? "active" : ""}" ${attr}="${escapeHtml(e.key)}" style="--c:${e.color}">
          <span class="cat-dot"></span><span class="side-label">${escapeHtml(e.label)}</span><span class="side-count">${e.count}</span>
        </button>`)
    .join("");

  function handlePaletteKeys(event, items, cursorKey, render, pick) {
    if (event.key === "ArrowDown" || event.key === "ArrowUp") {
      event.preventDefault();
      state[cursorKey] = (state[cursorKey] ?? 0) + (event.key === "ArrowDown" ? 1 : -1);
      render();
    } else if (event.key === "Enter" && event.target.tagName !== "BUTTON" && items[state[cursorKey]]) {
      event.preventDefault();
      pick(items[state[cursorKey]]);
    }
  }

  function filteredPresets() {
    const query = state.presetsSearch.trim().toLowerCase();
    return (state.presetsMeta || []).filter((item) => {
      if (state.presetsCategory === "__fav" && !item.favorite) return false;
      if (state.presetsCategory && state.presetsCategory !== "__fav" && item.category !== state.presetsCategory) return false;
      if (!query) return true;
      const hay = `${item.name} ${item.category} ${(item.tags || []).join(" ")}`.toLowerCase();
      return hay.includes(query);
    });
  }

  function renderPresetsList() {
    const root = $("presetsList");
    const side = $("presetsCategoryChips");
    if (!root) return;

    const all = state.presetsMeta || [];
    const categories = state.presetCategories || [];
    const categoryList = $("presetCategoryList");
    if (categoryList) categoryList.innerHTML = categories.map((c) => `<option value="${escapeHtml(c)}"></option>`).join("");

    if (side) {
      side.innerHTML = sideItemsHtml([
        { key: "", label: "All presets", count: all.length, color: "var(--text)" },
        { key: "__fav", label: "Favorites", count: all.filter((i) => i.favorite).length, color: "var(--amber)" },
        ...categories.map((c) => ({ key: c, label: c, count: all.filter((i) => i.category === c).length, color: categoryColor(c) })),
      ], state.presetsCategory, "data-preset-cat");
    }

    const items = filteredPresets();
    $("presetsCount").textContent = `${items.length} preset${items.length === 1 ? "" : "s"}`;
    state.presetsCursor = Math.min(Math.max(0, state.presetsCursor ?? 0), Math.max(0, items.length - 1));

    if (!items.length) {
      root.innerHTML = `
        <div class="palette-empty">
          <strong>No presets match</strong>
          Try another search or category, or save the current tone below.
        </div>`;
      return;
    }

    root.innerHTML = items
      .map((item, index) => {
        const active = state.currentPreset === item.name;
        const name = escapeHtml(item.name);
        return `
      <div class="palette-row ${active ? "active" : ""} ${index === state.presetsCursor ? "cursor" : ""}" role="option" aria-selected="${active}" style="--c:${categoryColor(item.category)}">
        <button type="button" class="palette-row-main" data-preset-load="${name}">
          <span class="cat-dot"></span>
          <span class="palette-row-name">${escapeHtml(displayName(item.name))}</span>
          <span class="palette-row-meta">${escapeHtml((item.tags || []).filter((tag) => tag !== "factory").slice(0, 3).join(", "))}</span>
        </button>
        <span class="palette-row-aside">
          <span class="row-loaded" title="${active ? "Loaded" : ""}">${active ? ICON_CHECK : ""}</span>
          <button type="button" class="row-icon" data-preset-export="${name}" title="Export">${ICON_EXPORT}</button>
          <button type="button" class="row-icon danger" data-preset-delete="${name}" title="Delete">${ICON_DELETE}</button>
          <button type="button" class="row-icon star ${item.favorite ? "on" : ""}" data-preset-star="${name}" title="Favorite">${ICON_STAR}</button>
        </span>
      </div>`;
      })
      .join("");
    root.querySelector(".cursor")?.scrollIntoView({ block: "nearest" });
  }

  function loadPresetAndClose(name) {
    send({ type: "loadPreset", name });
    closePopovers();
  }

  function handlePresetKeys(event) {
    if (event.target.closest(".palette-save")) {
      if (event.key === "Enter") $("btnSave")?.click();
      return;
    }
    handlePaletteKeys(event, filteredPresets(), "presetsCursor", renderPresetsList, (item) => loadPresetAndClose(item.name));
  }

  const libraryGroups = () => ({
    models: state.library.models,
    irs: state.library.irs,
    fav: [...state.library.models, ...state.library.irs].filter((item) => item.favorite),
  });

  function filteredLibraryItems() {
    const query = state.librarySearch.trim().toLowerCase();
    return (libraryGroups()[state.libraryTab] || []).filter((item) => String(item.name || "").toLowerCase().includes(query));
  }

  function isLibraryItemLoaded(item) {
    const [loaded, name] = item.kind === "ir" ? [state.irLoaded, state.irName] : [state.namLoaded, state.namName];
    // the engine reports the file name, the library lists it without the extension
    return loaded && item.name === String(name).replace(/\.\w+$/, "");
  }

  function loadResourceAndClose(item) {
    send({ type: "loadResource", kind: item.kind, path: item.path });
    closePopovers();
  }

  function renderLibraryList() {
    const root = $("libraryList");
    if (!root) return;

    const groups = libraryGroups();
    $("libraryTabs").innerHTML = sideItemsHtml([
      { key: "models", label: "Amp models", count: groups.models.length, color: "var(--sky)" },
      { key: "irs", label: "Cab IRs", count: groups.irs.length, color: "var(--mint)" },
      { key: "fav", label: "Favorites", count: groups.fav.length, color: "var(--amber)" },
    ], state.libraryTab, "data-library-tab");

    const items = filteredLibraryItems();
    $("libraryCount").textContent = `${items.length} item${items.length === 1 ? "" : "s"}`;
    state.libraryCursor = Math.min(Math.max(0, state.libraryCursor ?? 0), Math.max(0, items.length - 1));

    if (!items.length) {
      const fav = state.libraryTab === "fav";
      root.innerHTML = `
        <div class="palette-empty">
          <strong>${fav ? "No favorites yet" : "Nothing matches"}</strong>
          ${fav ? "Star an amp model or cab IR to keep it here." : "Try another search, or import a file."}
        </div>`;
      return;
    }

    root.innerHTML = items
      .map((item, index) => {
        const active = isLibraryItemLoaded(item);
        const path = escapeHtml(item.path);
        return `
      <div class="palette-row ${active ? "active" : ""} ${index === state.libraryCursor ? "cursor" : ""}" role="option" aria-selected="${active}" style="--c:var(--${item.kind === "ir" ? "mint" : "sky"})">
        <button type="button" class="palette-row-main" data-load-path="${path}" data-load-kind="${escapeHtml(item.kind)}">
          <span class="cat-dot"></span>
          <span class="palette-row-name">${escapeHtml(item.name)}</span>
          <span class="palette-row-meta">${item.source === "user" ? "Imported" : "Factory"}</span>
        </button>
        <span class="palette-row-aside">
          <span class="row-loaded" title="${active ? "Loaded" : ""}">${active ? ICON_CHECK : ""}</span>
          <button type="button" class="row-icon star ${item.favorite ? "on" : ""}" data-star-path="${path}" title="Favorite">${ICON_STAR}</button>
        </span>
      </div>`;
      })
      .join("");
    root.querySelector(".cursor")?.scrollIntoView({ block: "nearest" });
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
    bindToggle("eqEnabled", "eqEnabled");
    bindToggle("cabEnabled", "cabEnabled");
    bindToggle("metronomeEnabled", "metronomeEnabled");
    bindToggle("compressorEnabled", "compressorEnabled");
    bindToggle("driveEnabled", "driveEnabled");
    bindToggle("delayEnabled", "delayEnabled");
    bindToggle("delaySync", "delaySync");
    bindToggle("delayPingPong", "delayPingPong");
    bindToggle("doublerEnabled", "doublerEnabled");
    bindToggle("reverbEnabled", "reverbEnabled");
    bindChoiceGroups();

    $("presetSelect").addEventListener("change", (event) => {
      send({ type: "loadPreset", name: event.target.value });
    });
    $("btnSave")?.addEventListener("click", () => {
      const name =
        $("presetName")?.value.trim() ||
        state.currentPreset ||
        "";
      if (!name) {
        openPopover("presets");
        $("presetName")?.focus();
        return;
      }
      send({
        type: "savePreset",
        name,
        category: ($("presetCategory")?.value || "").trim(),
        tags: [],
      });
    });
    $("btnPresets")?.addEventListener("click", () => openPopover("presets"));
    $("btnPresetsClose")?.addEventListener("click", closePopovers);
    $("btnPresetImport")?.addEventListener("click", () => send({ type: "importPreset" }));
    $("presetsSearch")?.addEventListener("input", (event) => {
      state.presetsSearch = event.target.value || "";
      state.presetsCursor = 0;
      renderPresetsList();
    });
    $("presetsPopover")?.addEventListener("keydown", handlePresetKeys);
    document.addEventListener("keydown", (event) => {
      if (event.key === "Escape" && state.openPopover) closePopovers();
    });
    $("presetsCategoryChips")?.addEventListener("click", (event) => {
      const item = event.target.closest("[data-preset-cat]");
      if (!item) return;
      state.presetsCategory = item.getAttribute("data-preset-cat") || "";
      state.presetsCursor = 0;
      renderPresetsList();
    });
    $("presetsList")?.addEventListener("click", (event) => {
      const star = event.target.closest("[data-preset-star]");
      if (star) {
        send({ type: "togglePresetFavorite", name: star.getAttribute("data-preset-star") });
        return;
      }
      const exp = event.target.closest("[data-preset-export]");
      if (exp) {
        send({ type: "exportPreset", name: exp.getAttribute("data-preset-export") });
        return;
      }
      const del = event.target.closest("[data-preset-delete]");
      if (del) {
        const name = del.getAttribute("data-preset-delete");
        if (name && window.confirm(`Delete preset “${name}”?`))
          send({ type: "deletePreset", name });
        return;
      }
      const load = event.target.closest("[data-preset-load]");
      if (load) loadPresetAndClose(load.getAttribute("data-preset-load"));
    });
    const openLibrary = (tab) => {
      state.libraryTab = tab;
      openPopover("library");
    };
    $("btnBrowseNam").addEventListener("click", () => openLibrary("models"));
    $("btnBrowseIr").addEventListener("click", () => openLibrary("irs"));
    const openAudioSettings = () => send({ type: "openAudioSettings" });
    $("btnAudio")?.addEventListener("click", openAudioSettings);
    $("btnSettings")?.addEventListener("click", () => openPopover("settings"));
    $("btnSettingsClose")?.addEventListener("click", closePopovers);
    $("btnSettingsAudio")?.addEventListener("click", openAudioSettings);
    $("btnPresetExportPack")?.addEventListener("click", () => {
      const name = state.currentPreset || $("presetName")?.value || "";
      if (!name) {
        $("footerStatus").textContent = "Load or select a preset first";
        return;
      }
      send({ type: "exportPresetPack", name });
    });
    $("demoClipSelect")?.addEventListener("change", (event) => {
      const clipId = event.target.value || "";
      state.demo.activeClipId = clipId;
      const opt = event.target.selectedOptions && event.target.selectedOptions[0];
      if (opt) state.demo.activeClipName = opt.textContent || clipId;
      send({ type: "demoSelect", clipId });
    });
    $("demoClipSelect")?.addEventListener("mousedown", (event) => {
      // Keep the native dropdown usable inside the dock.
      event.stopPropagation();
    });
    $("btnDemoPlay")?.addEventListener("click", (event) => {
      event.preventDefault();
      event.stopPropagation();
      const clipId =
        state.demo.activeClipId
        || $("demoClipSelect")?.value
        || (state.demo.clips[0] && state.demo.clips[0].id)
        || "fallback";

      // Optimistic UI so the button responds even before the native bridge replies.
      state.demo.playing = !state.demo.playing;
      state.demo.activeClipId = clipId;
      renderDemoUi();
      send({ type: "demoToggle", clipId });
    });
    $("calibrationSelect")?.addEventListener("change", (event) => {
      send({ type: "setCalibration", id: event.target.value });
    });
    $("btnCalibrationSave")?.addEventListener("click", () => {
      const name = ($("calibrationName")?.value || state.settings.activeCalibrationName || "Custom").trim();
      send({
        type: "saveCalibration",
        name,
        gainDb: state.parameters.calibrationGain ?? 0,
      });
    });
    $("btnCalibrationDelete")?.addEventListener("click", () => {
      if (!state.settings.activeCalibrationId) return;
      if (window.confirm("Delete this calibration profile?"))
        send({ type: "deleteCalibration", id: state.settings.activeCalibrationId });
    });
    $("btnLibrary")?.addEventListener("click", () => openPopover("library"));
    $("btnLibraryClose")?.addEventListener("click", closePopovers);
    $("btnLibraryImport")?.addEventListener("click", () => {
      send({ type: "importResource", kind: state.libraryTab === "irs" ? "ir" : "nam" });
    });
    $("librarySearch")?.addEventListener("input", (event) => {
      state.librarySearch = event.target.value || "";
      state.libraryCursor = 0;
      renderLibraryList();
    });
    $("libraryTabs")?.addEventListener("click", (event) => {
      const tab = event.target.closest("[data-library-tab]");
      if (!tab) return;
      state.libraryTab = tab.getAttribute("data-library-tab");
      state.libraryCursor = 0;
      renderLibraryList();
    });
    $("libraryPopover")?.addEventListener("keydown", (event) =>
      handlePaletteKeys(event, filteredLibraryItems(), "libraryCursor", renderLibraryList, loadResourceAndClose));
    $("libraryList")?.addEventListener("click", (event) => {
      const star = event.target.closest("[data-star-path]");
      if (star) {
        send({ type: "toggleFavorite", path: star.getAttribute("data-star-path") });
        return;
      }
      const load = event.target.closest("[data-load-path]");
      if (load) loadResourceAndClose({ kind: load.getAttribute("data-load-kind"), path: load.getAttribute("data-load-path") });
    });

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
      if (state.openPopover === "library") renderLibraryList();
      if (state.openPopover === "presets") renderPresetsList();
    } else if (data.type === "meters") {
      paintMeter("input-meter", data.inputDb ?? -60);
      paintMeter("output-meter", data.outputDb ?? -60);
      state.cpuPercent = Number(data.cpuPercent) || 0;
      state.hotInput = !!data.hotInput;
      state.hotOutput = !!data.hotOutput;
      const cpuEl = $("dockCpu");
      if (cpuEl) {
        cpuEl.textContent = `CPU ${state.cpuPercent.toFixed(0)}%`;
        cpuEl.classList.toggle("high", state.cpuPercent >= 70);
      }
      const hotEl = $("dockHot");
      if (hotEl) {
        const hot = state.hotInput || state.hotOutput;
        hotEl.hidden = !hot;
        hotEl.textContent = state.hotInput && state.hotOutput
          ? "HOT I/O"
          : state.hotInput
            ? "HOT IN"
            : "HOT OUT";
      }
      const inputMeter = $("input-meter");
      const outputMeter = $("output-meter");
      if (inputMeter) inputMeter.classList.toggle("hot", !!state.hotInput);
      if (outputMeter) outputMeter.classList.toggle("hot", !!state.hotOutput);
    } else if (data.type === "practice") {
      paintPractice(data);
    } else if (data.type === "demo") {
      state.demo = {
        playing: !!data.playing,
        activeClipId: data.activeClipId || "",
        activeClipName: data.activeClipName || "",
        clips: Array.isArray(data.clips) ? data.clips : [],
      };
      renderDemoUi();
    } else if (data.type === "settings") {
      state.settings = {
        activeCalibrationId: data.activeCalibrationId || "",
        activeCalibrationName: data.activeCalibrationName || "",
        activeCalibrationGainDb: Number(data.activeCalibrationGainDb) || 0,
        calibrationProfiles: Array.isArray(data.calibrationProfiles) ? data.calibrationProfiles : [],
      };
      state.parameters.calibrationGain = state.settings.activeCalibrationGainDb;
      setKnob("calibrationGain", state.parameters.calibrationGain);
      renderCalibrationUi();
    } else if (data.type === "library") {
      state.library = {
        models: Array.isArray(data.models) ? data.models : [],
        irs: Array.isArray(data.irs) ? data.irs : [],
      };
      renderLibraryList();
    } else if (data.type === "presets") {
      state.presetsMeta = Array.isArray(data.items) ? data.items : [];
      state.presetCategories = Array.isArray(data.categories) ? data.categories : [];
      if (data.current) state.currentPreset = data.current;
      const names = state.presetsMeta.map((item) => item.name);
      state.presets = names;
      const select = $("presetSelect");
      if (select) {
        const previous = select.value;
        select.innerHTML = "";
        names.forEach((name) => {
          const opt = document.createElement("option");
          opt.value = name;
          opt.textContent = name;
          select.appendChild(opt);
        });
        if (state.currentPreset) select.value = state.currentPreset;
        else if (previous) select.value = previous;
      }
      const currentName = $("presetCurrentName");
      if (currentName) currentName.textContent = state.currentPreset ? displayName(state.currentPreset) : "No preset";
      renderPresetsList();
    } else if (data.type === "status") {
      $("footerStatus").textContent = data.message || "";
    }
  };

  buildMeters();
  bindKnobs();
  bindControls();
  bindStage();
  applyState(state);
  showSection("pedals");
  send({ type: "ready" });
})();
