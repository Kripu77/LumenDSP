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
    compressorThreshold: { min: -40, max: 0, unit: "dB", decimals: 1 },
    compressorRatio: { min: 1, max: 20, unit: ":1", decimals: 1 },
    compressorMix: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    driveAmount: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    driveTone: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    driveLevel: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    delayTime: { min: 1, max: 1000, unit: "ms", decimals: 0 },
    delayFeedback: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    delayMix: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    reverbSize: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    reverbDamping: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
    reverbMix: { min: 0, max: 1, unit: "%", decimals: 0, asPercent: true },
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
      reverbEnabled: false,
      reverbCharacter: 0,
      reverbSize: 0.4,
      reverbDamping: 0.5,
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
    libraryFavoritesOnly: false,
    library: { models: [], irs: [] },
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

  function focusStage(focusId) {
    if (!focusId) return;
    const el = document.querySelector(focusId);
    if (!el) return;
    el.scrollIntoView({ behavior: "smooth", block: "nearest", inline: "nearest" });
    el.classList.add("stage-flash");
    window.setTimeout(() => el.classList.remove("stage-flash"), 450);
  }

  function togglePathParam(paramId) {
    if (!paramId || state.parameters[paramId] === undefined) return;
    const next = !state.parameters[paramId];
    state.parameters[paramId] = next;
    setToggle(paramId, next);
    if (paramId === "noiseGateEnabled") setToggle("noiseGateEnabled", next);
    send({ type: "setParameter", id: paramId, value: next ? 1 : 0 });

    document.querySelectorAll(".fx-module").forEach((mod) => {
      const key = mod.getAttribute("data-fx");
      const enabled =
        key === "comp" ? state.parameters.compressorEnabled
        : key === "drive" ? state.parameters.driveEnabled
        : key === "delay" ? state.parameters.delayEnabled
        : key === "reverb" ? state.parameters.reverbEnabled
        : false;
      mod.classList.toggle("on", !!enabled);
    });

    if (paramId === "cabEnabled") {
      const cabLed = $("cabLed");
      if (cabLed) cabLed.classList.toggle("on", next && state.irLoaded);
    }
    updateFxModuleClasses();
    renderSignalPath();
  }

  function renderSignalPath() {
    const root = $("signalPath");
    if (!root) return;
    const p = state.parameters;
    const nodes = [
      {
        id: "gate",
        title: "GATE",
        detail: p.noiseGateEnabled ? `${Number(p.noiseGateThreshold).toFixed(0)} dB` : "Off",
        on: !!p.noiseGateEnabled,
        toggle: "noiseGateEnabled",
        focus: ".card-gate",
      },
      {
        id: "comp",
        title: "COMP",
        detail: p.compressorEnabled ? `${Number(p.compressorRatio).toFixed(1)}:1` : "Off",
        on: !!p.compressorEnabled,
        toggle: "compressorEnabled",
        focus: '.fx-module[data-fx="comp"]',
      },
      {
        id: "drive",
        title: "DRIVE",
        detail: p.driveEnabled
          ? ["Soft", "Hard", "Tube", "Boost"][Math.round(p.driveMode) || 0]
          : "Off",
        on: !!p.driveEnabled,
        toggle: "driveEnabled",
        focus: '.fx-module[data-fx="drive"]',
      },
      {
        id: "amp",
        title: "AMP",
        detail: state.namLoaded ? state.namName : "No model",
        on: state.namLoaded,
        cls: "amp",
        focus: ".card-amp",
      },
      {
        id: "eq",
        title: "EQ",
        detail: p.eqEnabled ? "Tone" : "Off",
        on: !!p.eqEnabled,
        toggle: "eqEnabled",
        focus: ".card-amp",
      },
      {
        id: "cab",
        title: "CAB",
        detail: state.irLoaded ? state.irName : "No IR",
        on: !!p.cabEnabled && state.irLoaded,
        toggle: "cabEnabled",
        focus: ".card-cab",
      },
      {
        id: "delay",
        title: "DELAY",
        detail: p.delayEnabled
          ? (p.delaySync
            ? ["1/4", "1/8", "1/8D", "1/16", "1/4D", "1/2"][Math.round(p.delayDivision) || 0]
            : `${Math.round(p.delayTime)} ms`)
          : "Off",
        on: !!p.delayEnabled,
        toggle: "delayEnabled",
        focus: '.fx-module[data-fx="delay"]',
      },
      {
        id: "reverb",
        title: "REVERB",
        detail: p.reverbEnabled
          ? ["Room", "Hall", "Plate", "Amb"][Math.round(p.reverbCharacter) || 0]
          : "Off",
        on: !!p.reverbEnabled,
        toggle: "reverbEnabled",
        focus: '.fx-module[data-fx="reverb"]',
      },
    ];

    root.innerHTML = nodes
      .map(
        (node) => `
      <button type="button"
        class="path-node ${node.cls || ""} ${node.on ? "on" : ""}"
        data-path-id="${node.id}"
        data-toggle="${node.toggle || ""}"
        data-focus="${node.focus || ""}"
        aria-pressed="${node.on ? "true" : "false"}"
        title="${node.toggle ? "Click to toggle · " : ""}${node.title}">
        <span class="path-title">${node.title}</span>
        <span class="path-detail">${escapeHtml(node.detail)}</span>
      </button>`
      )
      .join("");
  }

  function bindSignalPath() {
    const root = $("signalPath");
    if (!root || root.dataset.bound === "1") return;
    root.dataset.bound = "1";
    root.addEventListener("click", (event) => {
      const node = event.target.closest(".path-node");
      if (!node || !root.contains(node)) return;
      const toggle = node.getAttribute("data-toggle");
      const focus = node.getAttribute("data-focus");
      if (toggle) togglePathParam(toggle);
      focusStage(focus);
    });
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
    setToggle("compressorEnabled", state.parameters.compressorEnabled);
    setToggle("driveEnabled", state.parameters.driveEnabled);
    setToggle("delayEnabled", state.parameters.delayEnabled);
    setToggle("delaySync", state.parameters.delaySync);
    setToggle("reverbEnabled", state.parameters.reverbEnabled);

    syncChoiceChips("driveMode", state.parameters.driveMode);
    syncChoiceChips("delayDivision", state.parameters.delayDivision);
    syncChoiceChips("reverbCharacter", state.parameters.reverbCharacter);
    updateFxModuleClasses();

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
      setToggle(paramId === "noiseGateEnabled" ? "noiseGateEnabled" : paramId, next);
      if (paramId === "noiseGateEnabled") setToggle("noiseGateEnabled", next);
      send({ type: "setParameter", id: paramId, value: next ? 1 : 0 });
      renderSignalPath();
      updateFxModuleClasses();
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
    const library = $("libraryPopover");
    if (backdrop) backdrop.hidden = true;
    if (tuner) tuner.hidden = true;
    if (metro) metro.hidden = true;
    if (library) library.hidden = true;
    $("btnTuner")?.setAttribute("aria-pressed", "false");
    $("btnMetro")?.setAttribute("aria-pressed", "false");
    $("btnLibrary")?.setAttribute("aria-pressed", "false");
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
      renderLibraryList();
    }
  }

  function filteredLibraryItems() {
    const source = state.libraryTab === "irs" ? state.library.irs : state.library.models;
    const query = state.librarySearch.trim().toLowerCase();
    return source.filter((item) => {
      if (state.libraryFavoritesOnly && !item.favorite) return false;
      if (!query) return true;
      return String(item.name || "").toLowerCase().includes(query);
    });
  }

  function renderLibraryList() {
    const root = $("libraryList");
    const count = $("libraryCount");
    const subtitle = $("librarySubtitle");
    if (!root) return;

    const isIrs = state.libraryTab === "irs";
    const items = filteredLibraryItems();
    const total = isIrs ? state.library.irs.length : state.library.models.length;
    const favCount = (isIrs ? state.library.irs : state.library.models).filter((i) => i.favorite).length;

    if (count) {
      count.textContent = state.libraryFavoritesOnly
        ? `${items.length} favorite${items.length === 1 ? "" : "s"}`
        : `${items.length} of ${total}`;
    }
    if (subtitle) {
      subtitle.textContent = isIrs
        ? "Cabinet impulse responses"
        : "Neural amp models";
    }

    if (!items.length) {
      const label = isIrs ? "IRs" : "models";
      root.innerHTML = `
        <div class="library-empty">
          <strong>No matching ${label}</strong>
          ${state.libraryFavoritesOnly
            ? "Star items to build a favorites list, or turn off the filter."
            : "Try another search, or import a file into your library."}
        </div>`;
      return;
    }

    const activeName = isIrs
      ? (state.irLoaded ? state.irName : "")
      : (state.namLoaded ? state.namName : "");

    const favorites = items.filter((item) => item.favorite);
    const rest = items.filter((item) => !item.favorite);
    const groups = [];
    if (favorites.length && !state.libraryFavoritesOnly)
      groups.push({ label: "Favorites", items: favorites });
    if (rest.length)
      groups.push({
        label: state.libraryFavoritesOnly ? "Favorites" : favorites.length ? "All" : "",
        items: rest.length ? rest : favorites,
      });
    if (state.libraryFavoritesOnly)
      groups.splice(0, groups.length, { label: "", items });

    const typeLabel = isIrs ? "IR" : "NAM";
    const typeClass = isIrs ? "ir" : "nam";

    root.innerHTML = groups
      .map((group) => {
        const heading = group.label
          ? `<div class="library-section-label">${group.label}</div>`
          : "";
        const rows = group.items
          .map((item) => {
            const active = activeName && item.name === activeName;
            return `
      <div class="library-item ${active ? "active" : ""}" role="option">
        <button type="button" class="library-star ${item.favorite ? "on" : ""}" data-star-path="${escapeHtml(item.path)}" title="Favorite">${item.favorite ? "★" : "☆"}</button>
        <div class="library-type ${typeClass}" aria-hidden="true">${typeLabel}</div>
        <button type="button" class="library-item-main" data-load-path="${escapeHtml(item.path)}" data-load-kind="${item.kind}">
          <span class="library-item-name">${escapeHtml(item.name)}</span>
          <span class="library-item-meta">${item.source === "user" ? "Imported to your library" : "Factory content"}</span>
        </button>
        <div class="library-item-aside">
          ${active ? `<span class="library-loaded">Loaded</span>` : ""}
          <span class="library-badge ${item.source === "user" ? "user" : ""}">${item.source === "user" ? "User" : "Factory"}</span>
        </div>
      </div>`;
          })
          .join("");
        return heading + rows;
      })
      .join("");
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
    bindToggle("compressorEnabled", "compressorEnabled");
    bindToggle("driveEnabled", "driveEnabled");
    bindToggle("delayEnabled", "delayEnabled");
    bindToggle("delaySync", "delaySync");
    bindToggle("reverbEnabled", "reverbEnabled");
    bindChoiceGroups();

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
    $("btnBrowseNam").addEventListener("click", () => openPopover("library"));
    $("btnBrowseIr").addEventListener("click", () => {
      state.libraryTab = "irs";
      document.querySelectorAll("[data-library-tab]").forEach((btn) => {
        btn.classList.toggle("active", btn.getAttribute("data-library-tab") === "irs");
      });
      openPopover("library");
    });
    $("btnAudio")?.addEventListener("click", () => send({ type: "openAudioSettings" }));
    $("btnLibrary")?.addEventListener("click", () => openPopover("library"));
    $("btnLibraryClose")?.addEventListener("click", closePopovers);
    $("btnLibraryImport")?.addEventListener("click", () => {
      send({ type: "importResource", kind: state.libraryTab === "irs" ? "ir" : "nam" });
    });
    $("librarySearch")?.addEventListener("input", (event) => {
      state.librarySearch = event.target.value || "";
      renderLibraryList();
    });
    $("libraryFavoritesOnly")?.addEventListener("click", () => {
      state.libraryFavoritesOnly = !state.libraryFavoritesOnly;
      $("libraryFavoritesOnly")?.setAttribute(
        "aria-pressed",
        state.libraryFavoritesOnly ? "true" : "false"
      );
      renderLibraryList();
    });
    document.querySelectorAll("[data-library-tab]").forEach((btn) => {
      btn.addEventListener("click", () => {
        state.libraryTab = btn.getAttribute("data-library-tab") || "models";
        document.querySelectorAll("[data-library-tab]").forEach((el) => {
          el.classList.toggle("active", el === btn);
        });
        renderLibraryList();
      });
    });
    $("libraryList")?.addEventListener("click", (event) => {
      const star = event.target.closest("[data-star-path]");
      if (star) {
        event.preventDefault();
        send({ type: "toggleFavorite", path: star.getAttribute("data-star-path") });
        return;
      }
      const load = event.target.closest("[data-load-path]");
      if (load) {
        send({
          type: "loadResource",
          kind: load.getAttribute("data-load-kind"),
          path: load.getAttribute("data-load-path"),
        });
      }
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
    } else if (data.type === "meters") {
      paintMeter("input-meter", data.inputDb ?? -60);
      paintMeter("output-meter", data.outputDb ?? -60);
    } else if (data.type === "practice") {
      paintPractice(data);
    } else if (data.type === "library") {
      state.library = {
        models: Array.isArray(data.models) ? data.models : [],
        irs: Array.isArray(data.irs) ? data.irs : [],
      };
      renderLibraryList();
    } else if (data.type === "status") {
      $("footerStatus").textContent = data.message || "";
    }
  };

  buildMeters();
  bindKnobs();
  bindControls();
  bindSignalPath();
  applyState(state);
  send({ type: "ready" });
})();
