// Practice studio UI: timing, vibrato, fretboard and log panels.
//
// Analysis lives in the sibling modules (timing.js, vibrato.js, ...); this file only wires
// them to the engine bridge and paints results. app.js calls init() with a small bridge
// before it announces "ready", and forwards every practice* message to receive().
(function attachPracticeUi(root) {
  "use strict";

  const practice = root.LumenPractice;
  const { events, timing, vibrato, fretboard, log: practiceLog } = practice;

  const MILLISECONDS_PER_SECOND = 1000;
  const SAVE_DEBOUNCE_MS = 400;
  const CALIBRATION_CLICKS = 12;
  // Ignore the first clicks of a calibration: the player is still finding the pulse.
  const CALIBRATION_WARMUP_CLICKS = 2;
  const MINIMUM_HITS_TO_LOG = 8;
  // How long to wait for the engine to confirm a stop (and flush its last events)
  // before finishing the take anyway, e.g. if the bridge is gone.
  const STOP_CONFIRM_TIMEOUT_MS = 600;
  // The tuner analyses 2048-sample windows; see vibrato.js analysisWindowMs.
  const TUNER_WINDOW_SAMPLES = 2048;
  const QUIZ_ROUND_LENGTH = 10;
  const LIVE_TRACE_WINDOW_MS = 4000;
  // Re-analyse the whole take this often while recording, for live vibrato numbers.
  const LIVE_ANALYSIS_INTERVAL_MS = 500;
  const RECENT_SESSION_COUNT = 12;
  const TREND_DAYS = 30;
  const NECK_FRET_COUNT = 22;
  const SVG_NS_ATTR = 'xmlns="http://www.w3.org/2000/svg"';

  const SUBDIVISIONS = Object.freeze([
    { value: 1, label: "1/4", counts: ["1"] },
    { value: 2, label: "1/8", counts: ["1", "&"] },
    { value: 3, label: "trip", counts: ["1", "trip", "let"] },
    { value: 4, label: "1/16", counts: ["1", "e", "&", "a"] },
    { value: 6, label: "sext", counts: ["1", "la", "li", "&", "la", "li"] },
  ]);
  const TOLERANCES_MS = Object.freeze([10, 15, 25]);
  const INLAY_FRETS = Object.freeze([3, 5, 7, 9, 15, 17, 19, 21]);
  const DOUBLE_INLAY_FRET = 12;

  const ui = {
    tool: "timing",
    log: practiceLog.createEmptyLog(),
    capture: { enabled: false, sampleRate: 48000, deviceLatencySamples: 0 },
    take: null,
    takeMode: null,
    isStopping: false,
    takeStartedAt: 0,
    lastTimingTake: null,
    restoreMetronomeTo: null,
    timingResult: null,
    calibrationResult: null,
    vibratoResult: null,
    liveTrace: [],
    beatInBar: -1,
    board: { rootName: "E", scaleId: "lydian", tuningId: "standard", minFret: 0, maxFret: 12, labels: "degrees" },
    quiz: null,
  };

  let bridge = null;
  let saveTimer = 0;
  let lastLiveAnalysisAt = 0;
  let stopTimer = 0;
  const $ = (id) => document.getElementById(id);

  // ---- helpers ------------------------------------------------------------------------

  function escapeHtml(text) {
    return String(text).replaceAll("&", "&amp;").replaceAll("<", "&lt;").replaceAll(">", "&gt;").replaceAll('"', "&quot;");
  }

  function formatSigned(value, decimals = 1) {
    const rounded = Number(value).toFixed(decimals);
    return value > 0 ? `+${rounded}` : rounded;
  }

  function setText(id, text) {
    const element = $(id);
    if (element) element.textContent = text;
  }

  function deviceLatencyMs() {
    const { deviceLatencySamples, sampleRate } = ui.capture;
    return sampleRate > 0 ? (deviceLatencySamples / sampleRate) * MILLISECONDS_PER_SECOND : 0;
  }

  function totalLatencyMs() {
    return deviceLatencyMs() + ui.log.settings.latencyOffsetMs;
  }

  function isCapturing() {
    return ui.take !== null;
  }

  function scheduleSave() {
    clearTimeout(saveTimer);
    saveTimer = setTimeout(() => bridge?.send({ type: "savePracticeLog", log: ui.log }), SAVE_DEBOUNCE_MS);
  }

  function updateLog(nextLog) {
    ui.log = nextLog;
    scheduleSave();
    renderLog();
    renderTargets();
  }

  function recordSession(kind, metrics, note = "") {
    const durationSec = Math.round((Date.now() - ui.takeStartedAt) / MILLISECONDS_PER_SECOND);
    updateLog(
      practiceLog.addSession(ui.log, {
        id: `${kind}-${Date.now()}`,
        kind,
        startedAt: new Date(ui.takeStartedAt || Date.now()).toISOString(),
        durationSec,
        metrics,
        note,
      })
    );
  }

  // ---- tool switching -------------------------------------------------------------------

  function showTool(tool) {
    if (tool === ui.tool && document.querySelector(`[data-practice-panel="${tool}"]:not([hidden])`)) return;
    if (isCapturing()) stopTake();
    ui.tool = tool;
    document.querySelectorAll("[data-practice-tool]").forEach((button) => {
      button.setAttribute("aria-pressed", String(button.dataset.practiceTool === tool));
    });
    document.querySelectorAll("[data-practice-panel]").forEach((panel) => {
      panel.hidden = panel.dataset.practicePanel !== tool;
    });
    if (tool === "fretboard") renderFretboard();
    if (tool === "log") renderLog();
  }

  // ---- capture ----------------------------------------------------------------------------

  function startTake(mode) {
    if (!bridge || isCapturing()) return;
    clearTimeout(stopTimer);
    ui.isStopping = false;
    ui.take = events.createTake();
    ui.takeMode = mode;
    ui.takeStartedAt = Date.now();
    ui.liveTrace = [];
    lastLiveAnalysisAt = 0;

    const needsClick = mode === "timing" || mode === "calibrate";
    if (needsClick && !bridge.getParameter("metronomeEnabled")) {
      ui.restoreMetronomeTo = false;
      bridge.setMetronomeEnabled(true);
    }

    bridge.send({ type: "practiceCapture", enabled: true });
    renderTakeState();
  }

  // Stopping is a handshake: the engine flushes its queue and then reports capture off,
  // and only then is the take finished, so its final notes are not lost.
  function stopTake() {
    if (!isCapturing() || ui.isStopping) return;
    ui.isStopping = true;
    bridge?.send({ type: "practiceCapture", enabled: false });
    stopTimer = setTimeout(finishTake, STOP_CONFIRM_TIMEOUT_MS);
    renderTakeState();
  }

  function finishTake() {
    clearTimeout(stopTimer);
    if (!isCapturing()) return;
    ui.isStopping = false;
    // Only undo the metronome change we made; respect it if the player turned it off meanwhile.
    if (ui.restoreMetronomeTo !== null && bridge?.getParameter("metronomeEnabled")) bridge.setMetronomeEnabled(ui.restoreMetronomeTo);
    ui.restoreMetronomeTo = null;

    const take = ui.take;
    const mode = ui.takeMode;
    ui.take = null;
    ui.takeMode = null;

    if (take.isInterrupted) {
      const message = "audio device changed during the take, so it was discarded";
      if (mode === "calibrate") setText("calibrationStatus", message);
      else if (mode === "timing") setText("timingTendency", message);
      else ui.vibratoResult = null;
      renderTakeState();
      return;
    }

    if (mode === "timing") finishTimingTake(take);
    else if (mode === "calibrate") finishCalibration(take);
    else if (mode === "vibrato") finishVibratoTake(take);
    renderTakeState();
  }

  function toggleTake(mode) {
    if (isCapturing()) stopTake();
    else startTake(mode);
  }

  function renderTakeState() {
    const capturing = isCapturing();
    document.querySelectorAll("[data-take]").forEach((button) => {
      const isThisTake = capturing && ui.takeMode === button.dataset.take;
      button.classList.toggle("recording", isThisTake);
      button.disabled = capturing && !isThisTake;
      const label = button.querySelector(".take-label");
      if (label) label.textContent = isThisTake ? (ui.isStopping ? "finishing" : "stop") : button.dataset.idleLabel;
    });
    $("practiceScene")?.classList.toggle("is-capturing", capturing);
  }

  function onEventsBatch(batch) {
    if (!isCapturing()) return;
    const pitchCountBefore = ui.take.pitchFrames.length;
    events.appendBatch(ui.take, batch);
    if (ui.take.isInterrupted) {
      stopTake();
      return;
    }

    if (ui.takeMode === "timing") {
      ui.timingResult = analyseCurrentTiming(ui.take);
      renderTimingResult(true);
    } else if (ui.takeMode === "calibrate") {
      setText("calibrationStatus", `click ${Math.min(ui.take.beats.length, CALIBRATION_CLICKS)} of ${CALIBRATION_CLICKS}: pick on every click`);
      // Wait for one click past the last so the pick on click N has arrived.
      if (ui.take.beats.length > CALIBRATION_CLICKS && !ui.isStopping) stopTake();
    } else if (ui.takeMode === "vibrato") {
      appendLiveTrace(ui.take, pitchCountBefore);
      renderLiveTrace();
      const now = Date.now();
      if (now - lastLiveAnalysisAt >= LIVE_ANALYSIS_INTERVAL_MS) {
        lastLiveAnalysisAt = now;
        ui.vibratoResult = analyseVibratoTake(ui.take);
        renderVibratoResult();
      }
    }
  }

  // ---- timing -----------------------------------------------------------------------------

  function timingTargets() {
    return ui.log.targets.timing;
  }

  function analyseCurrentTiming(take) {
    return timing.analyseTiming(take.beats, take.onsets, {
      sampleRate: take.sampleRate,
      subdivision: timingTargets().subdivision,
      latencyMs: totalLatencyMs(),
      toleranceMs: timingTargets().toleranceMs,
    });
  }

  function finishTimingTake(take) {
    ui.lastTimingTake = take;
    ui.timingResult = analyseCurrentTiming(take);
    renderTimingResult(false);
    const result = ui.timingResult;
    if (!result.isValid || result.hits.length < MINIMUM_HITS_TO_LOG) return;
    recordSession("timing", {
      score: result.score,
      bpm: Math.round(bridge.getParameter("metronomeBpm")),
      subdivision: timingTargets().subdivision,
      meanDeviationMs: Number(result.meanDeviationMs.toFixed(1)),
      spreadMs: Number(result.standardDeviationMs.toFixed(1)),
      tightPercent: Math.round(result.tightPercent),
      tendency: result.tendency,
      hits: result.hits.length,
    });
  }

  // Lets the player re-read the same take under a different grid or tolerance (not re-logged).
  function reanalyseLastTimingTake() {
    if (isCapturing() || !ui.lastTimingTake) return;
    ui.timingResult = analyseCurrentTiming(ui.lastTimingTake);
    renderTimingResult(false);
  }

  function finishCalibration(take) {
    // Drop the warm-up clicks so the first few hesitant picks don't bias the median.
    // Onsets arrive late by the latency and a player may be early, so compare corrected
    // times against half a beat before the first kept click.
    const beats = take.beats.slice(CALIBRATION_WARMUP_CLICKS);
    const halfBeat = beats.length > 1 ? (beats[1].sampleIndex - beats[0].sampleIndex) / 2 : 0;
    const latencySamples = events.msToSamples(deviceLatencyMs(), take.sampleRate);
    const cutoff = (beats[0]?.sampleIndex ?? 0) - halfBeat;
    const onsets = take.onsets.filter((onset) => onset.sampleIndex - latencySamples >= cutoff);
    const estimate = timing.estimateLatencyMs(beats, onsets, { sampleRate: take.sampleRate, baseLatencyMs: deviceLatencyMs() });
    ui.calibrationResult = estimate;
    if (estimate.isValid) {
      updateLog(practiceLog.setSettings(ui.log, { latencyOffsetMs: Number((estimate.latencyMs - deviceLatencyMs()).toFixed(1)) }));
      setText("calibrationStatus", `calibrated from ${estimate.hitCount} picks (spread ${estimate.spreadMs.toFixed(1)} ms)`);
    } else {
      setText("calibrationStatus", `only heard ${estimate.hitCount} picks; try again a little louder`);
    }
    renderLatency();
  }

  function renderLatency() {
    setText("latencyDevice", `${deviceLatencyMs().toFixed(1)} ms`);
    setText("latencyOffset", `${formatSigned(ui.log.settings.latencyOffsetMs)} ms`);
    setText("latencyTotal", `${totalLatencyMs().toFixed(1)} ms`);
  }

  function renderTimingControls() {
    const { subdivision, toleranceMs } = timingTargets();
    $("subdivisionChips").innerHTML = SUBDIVISIONS.map(
      (option) =>
        `<button type="button" class="pr-chip" data-subdivision="${option.value}" aria-pressed="${option.value === subdivision}">${option.label}</button>`
    ).join("");
    $("toleranceChips").innerHTML = TOLERANCES_MS.map(
      (value) => `<button type="button" class="pr-chip" data-tolerance="${value}" aria-pressed="${value === toleranceMs}">±${value}</button>`
    ).join("");
    setText("practiceBpm", Math.round(bridge?.getParameter("metronomeBpm") ?? 120));
  }

  function renderTimingResult(isLive) {
    const result = ui.timingResult;
    const panel = $("timingResult");
    if (!panel) return;
    panel.classList.toggle("live", isLive);

    if (!result || !result.isValid) {
      setText("timingScore", "--");
      setText("timingMean", "--");
      setText("timingSpread", "--");
      setText("timingTight", "--");
      setText("timingTendency", result?.reason || "start a take and play along with the click");
      $("timingChart").innerHTML = "";
      $("timingSlots").innerHTML = "";
      return;
    }

    setText("timingScore", String(result.score));
    setText("timingMean", `${formatSigned(result.meanDeviationMs)} ms`);
    setText("timingSpread", `±${result.standardDeviationMs.toFixed(1)} ms`);
    setText("timingTight", `${Math.round(result.tightPercent)}%`);
    const extras = result.extraOnsetCount ? ` · ${result.extraOnsetCount} extra` : "";
    const dropped = (ui.take || ui.lastTimingTake)?.droppedEvents ? " · some events were dropped (CPU busy), result may be incomplete" : "";
    setText("timingTendency", `${result.tendency} · ${result.hits.length} notes${extras}${dropped}`);
    $("timingTendency").dataset.tendency = result.tendency;
    paintTimingChart();
    $("timingSlots").innerHTML = slotBreakdownHtml(result);
  }

  // Drawn at the container's real pixel size so dots stay round at any window size.
  function paintTimingChart() {
    const container = $("timingChart");
    const result = ui.timingResult;
    if (!container || !result?.isValid) return;
    const width = Math.max(320, container.clientWidth);
    const height = Math.max(120, container.clientHeight);
    container.innerHTML = timingScatterSvg(result, timingTargets().toleranceMs, width, height);
  }

  function timingScatterSvg(result, toleranceMs, width, height) {
    const padding = 22;
    const labelGutter = 80;
    const maxAbs = Math.max(40, Math.ceil(Math.max(...result.hits.map((hit) => Math.abs(hit.deviationMs))) / 10) * 10);
    const y = (ms) => height / 2 - (ms / maxAbs) * (height / 2 - padding / 2);
    const count = Math.max(1, result.hits.length - 1);
    const x = (index) => labelGutter + (index / count) * (width - labelGutter - padding);

    const band = `<rect x="${labelGutter}" y="${y(toleranceMs)}" width="${width - labelGutter - padding}" height="${y(-toleranceMs) - y(toleranceMs)}" class="pr-band"/>`;
    const axis = `<line x1="${labelGutter}" x2="${width - padding}" y1="${y(0)}" y2="${y(0)}" class="pr-axis"/>`;
    const labels = [
      `<text x="4" y="${y(maxAbs) + 4}" class="pr-tick">late ${maxAbs} ms</text>`,
      `<text x="4" y="${y(0) + 3}" class="pr-tick">on the click</text>`,
      `<text x="4" y="${y(-maxAbs)}" class="pr-tick">early ${maxAbs} ms</text>`,
    ].join("");
    const dots = result.hits
      .map((hit, index) => {
        const isTight = Math.abs(hit.deviationMs) <= toleranceMs;
        const tone = isTight ? "tight" : hit.deviationMs > 0 ? "late" : "early";
        return `<circle cx="${x(index).toFixed(1)}" cy="${y(hit.deviationMs).toFixed(1)}" r="${hit.slotInBeat === 0 ? 4.5 : 3.2}" class="pr-dot ${tone}"><title>${formatSigned(hit.deviationMs)} ms</title></circle>`;
      })
      .join("");
    return `<svg ${SVG_NS_ATTR} viewBox="0 0 ${width} ${height}" role="img" aria-label="Deviation of each note from the grid">${band}${axis}${labels}${dots}</svg>`;
  }

  function slotBreakdownHtml(result) {
    const counts = SUBDIVISIONS.find((option) => option.value === result.bySlotInBeat.length)?.counts || [];
    const maxAbs = Math.max(10, ...result.bySlotInBeat.map((slot) => Math.abs(slot.meanDeviationMs)));
    return result.bySlotInBeat
      .map((slot, index) => {
        const percent = (Math.abs(slot.meanDeviationMs) / maxAbs) * 50;
        const side = slot.meanDeviationMs >= 0 ? "late" : "early";
        return `<div class="pr-slot">
          <span class="pr-slot-count">${escapeHtml(counts[index] ?? index + 1)}</span>
          <span class="pr-slot-track"><span class="pr-slot-bar ${side}" style="--size:${percent.toFixed(1)}%"></span></span>
          <span class="pr-slot-value">${slot.count ? `${formatSigned(slot.meanDeviationMs)}` : "--"}</span>
        </div>`;
      })
      .join("");
  }

  // ---- vibrato ----------------------------------------------------------------------------

  function appendLiveTrace(take, fromIndex) {
    const origin = take.pitchFrames[0]?.sampleIndex ?? 0;
    for (const frame of take.pitchFrames.slice(fromIndex)) {
      const timeMs = events.samplesToMs(frame.sampleIndex - origin, take.sampleRate);
      const cents = frame.frequencyHz > 0 ? vibrato.hzToCents(frame.frequencyHz) : null;
      ui.liveTrace.push({ timeMs, cents });
    }
    const cutoff = (ui.liveTrace[ui.liveTrace.length - 1]?.timeMs ?? 0) - LIVE_TRACE_WINDOW_MS;
    ui.liveTrace = ui.liveTrace.filter((point) => point.timeMs >= cutoff);
  }

  function renderLiveTrace() {
    const width = 640;
    const height = 140;
    const rangeCents = 100;
    const trace = ui.liveTrace;
    if (trace.length < 2) {
      $("vibratoLive").innerHTML = "";
      return;
    }
    const end = trace[trace.length - 1].timeMs;
    const x = (timeMs) => width - ((end - timeMs) / LIVE_TRACE_WINDOW_MS) * width;
    // Show pitch relative to the nearest semitone so held notes sit on the centre line.
    const y = (cents) => {
      const offset = cents - 100 * Math.round(cents / 100);
      return height / 2 - (offset / rangeCents) * (height / 2);
    };
    let path = "";
    let isPenDown = false;
    for (const point of trace) {
      if (point.cents === null) {
        isPenDown = false;
        continue;
      }
      path += `${isPenDown ? "L" : "M"}${x(point.timeMs).toFixed(1)} ${y(point.cents).toFixed(1)}`;
      isPenDown = true;
    }
    const latest = [...trace].reverse().find((point) => point.cents !== null);
    setText("vibratoLiveNote", latest ? vibrato.noteNameForCents(latest.cents) : "--");
    $("vibratoLive").innerHTML = `<svg ${SVG_NS_ATTR} viewBox="0 0 ${width} ${height}" preserveAspectRatio="none" role="img" aria-label="Live pitch">
      <line x1="0" x2="${width}" y1="${height / 2}" y2="${height / 2}" class="pr-axis"/>
      <path d="${path}" class="pr-trace"/></svg>`;
  }

  function analyseVibratoTake(take) {
    const analysisWindowMs = take.sampleRate > 0 ? (TUNER_WINDOW_SAMPLES / take.sampleRate) * MILLISECONDS_PER_SECOND : 0;
    return vibrato.analyseVibrato(events.pitchFramesInMs(take), { analysisWindowMs });
  }

  function finishVibratoTake(take) {
    ui.vibratoResult = analyseVibratoTake(take);
    renderVibratoResult();
    const summary = ui.vibratoResult.summary;
    if (!summary) return;
    recordSession("vibrato", {
      score: vibrato.scoreAgainstTarget(summary, ui.log.targets.vibrato),
      rateHz: Number(summary.rateHz.toFixed(2)),
      widthCents: Math.round(summary.widthCents),
      consistency: summary.consistency,
      notes: summary.noteCount,
    });
  }

  function renderVibratoResult() {
    const result = ui.vibratoResult;
    const list = $("vibratoNotes");
    if (!list) return;
    const target = ui.log.targets.vibrato;

    if (!result || result.notes.length === 0) {
      setText("vibratoScore", "--");
      setText("vibratoRate", "--");
      setText("vibratoWidth", "--");
      setText("vibratoConsistency", "--");
      list.innerHTML = `<p class="pr-empty">${result ? "no sustained notes heard; hold each note for at least half a second" : "hold a note and add vibrato, then stop the take"}</p>`;
      return;
    }

    const summary = result.summary;
    setText("vibratoScore", summary ? String(vibrato.scoreAgainstTarget(summary, target)) : "--");
    setText("vibratoRate", summary ? `${summary.rateHz.toFixed(1)} Hz` : "--");
    setText("vibratoWidth", summary ? `${Math.round(summary.widthCents)} ¢` : "--");
    setText("vibratoConsistency", summary ? `${summary.consistency}` : "--");

    list.innerHTML = result.notes.map((note) => vibratoNoteHtml(note, target)).join("");
  }

  function vibratoNoteHtml(note, target) {
    const intonation = `${formatSigned(note.intonationCents, 0)}¢`;
    if (!note.hasVibrato) {
      return `<article class="pr-note straight"><header><strong>${escapeHtml(note.noteName)}</strong><span>straight · ${intonation}</span></header></article>`;
    }
    return `<article class="pr-note">
      <header><strong>${escapeHtml(note.noteName)}</strong><span>${note.rateHz.toFixed(1)} Hz · ${Math.round(note.widthCents)}¢ · even ${note.consistency} · ${intonation}</span></header>
      ${vibratoCurveSvg(note.curve, target.widthCents)}
    </article>`;
  }

  function vibratoCurveSvg(curve, targetWidthCents) {
    const width = 300;
    const height = 70;
    const peak = Math.max(targetWidthCents, ...curve.map((point) => Math.abs(point.cents) * 2)) / 2 || 1;
    const end = curve[curve.length - 1].timeMs || 1;
    const x = (timeMs) => (timeMs / end) * width;
    const y = (cents) => height / 2 - (cents / peak) * (height / 2 - 4);
    const path = curve.map((point, index) => `${index ? "L" : "M"}${x(point.timeMs).toFixed(1)} ${y(point.cents).toFixed(1)}`).join("");
    const half = targetWidthCents / 2;
    return `<svg ${SVG_NS_ATTR} viewBox="0 0 ${width} ${height}" preserveAspectRatio="none" role="img" aria-label="Vibrato shape against target width">
      <rect x="0" y="${y(half)}" width="${width}" height="${y(-half) - y(half)}" class="pr-band"/>
      <path d="${path}" class="pr-trace"/></svg>`;
  }

  // ---- targets ------------------------------------------------------------------------------

  function renderTargets() {
    const { rateHz, widthCents } = ui.log.targets.vibrato;
    setText("targetRate", `${rateHz.toFixed(1)} Hz`);
    setText("targetWidth", `${Math.round(widthCents)} ¢`);
    renderLatency();
    if (bridge) renderTimingControls();
  }

  function nudgeVibratoTarget(field, delta) {
    const limits = { rateHz: [2, 10], widthCents: [10, 200] };
    const [min, max] = limits[field];
    const current = ui.log.targets.vibrato[field];
    const next = Math.min(max, Math.max(min, Math.round((current + delta) * 10) / 10));
    updateLog(practiceLog.setTargets(ui.log, "vibrato", { [field]: next }));
    renderVibratoResult();
  }

  // ---- fretboard ----------------------------------------------------------------------------

  function renderFretboardControls() {
    const { rootName, scaleId, tuningId, minFret, maxFret, labels } = ui.board;
    $("boardRoot").innerHTML = fretboard.ROOTS.map((root) => `<option ${root === rootName ? "selected" : ""}>${root}</option>`).join("");

    const families = new Map();
    for (const [id, scale] of Object.entries(fretboard.SCALES)) {
      if (!families.has(scale.family)) families.set(scale.family, []);
      families.get(scale.family).push(`<option value="${id}" ${id === scaleId ? "selected" : ""}>${escapeHtml(scale.label)}</option>`);
    }
    $("boardScale").innerHTML = [...families].map(([family, options]) => `<optgroup label="${escapeHtml(family)}">${options.join("")}</optgroup>`).join("");

    $("boardTuning").innerHTML = Object.entries(fretboard.TUNINGS)
      .map(([id, tuning]) => `<option value="${id}" ${id === tuningId ? "selected" : ""}>${escapeHtml(tuning.label)}</option>`)
      .join("");
    $("boardMinFret").value = String(minFret);
    $("boardMaxFret").value = String(maxFret);
    document.querySelectorAll("[data-board-labels]").forEach((chip) => chip.setAttribute("aria-pressed", String(chip.dataset.boardLabels === labels)));
  }

  function fretX(fret, width) {
    // Real fret spacing (each fret 2^(-1/12) of the previous), scaled to fill the width.
    const scaleLength = 1 / (1 - 2 ** (-NECK_FRET_COUNT / 12));
    return width * scaleLength * (1 - 2 ** (-fret / 12));
  }

  function renderFretboard() {
    const { rootName, scaleId, tuningId, minFret, maxFret, labels } = ui.board;
    const board = fretboard.buildFretboard({ rootName, scaleId, tuningId, fretCount: NECK_FRET_COUNT });
    const scale = fretboard.SCALES[scaleId];
    const colourDegrees = new Set(scale.colour);
    const quiz = ui.quiz?.question;
    const isHidingLabels = Boolean(quiz);

    const width = 1000;
    const nutX = 34;
    const neckWidth = width - nutX - 12;
    const stringGap = 26;
    const top = 22;
    const height = top * 2 + stringGap * (board.length - 1);
    const xFor = (fret) => nutX + fretX(fret, neckWidth);
    const centreFor = (fret) => (fret === 0 ? nutX - 16 : (xFor(fret - 1) + xFor(fret)) / 2);
    // Board index 0 is the low string; draw it at the bottom like tab.
    const yFor = (stringIndex) => top + (board.length - 1 - stringIndex) * stringGap;

    const parts = [];
    parts.push(`<rect x="${nutX}" y="${top - 12}" width="${neckWidth}" height="${height - top * 2 + 24}" rx="4" class="fb-wood"/>`);
    for (const fret of INLAY_FRETS) parts.push(`<circle cx="${centreFor(fret)}" cy="${height / 2}" r="5" class="fb-inlay"/>`);
    parts.push(`<circle cx="${centreFor(DOUBLE_INLAY_FRET)}" cy="${yFor(1) + stringGap / 2}" r="5" class="fb-inlay"/><circle cx="${centreFor(DOUBLE_INLAY_FRET)}" cy="${yFor(4) - stringGap / 2}" r="5" class="fb-inlay"/>`);
    for (let fret = 1; fret <= NECK_FRET_COUNT; fret += 1) {
      parts.push(`<line x1="${xFor(fret)}" x2="${xFor(fret)}" y1="${top - 12}" y2="${height - top + 12}" class="fb-fret"/>`);
      if (INLAY_FRETS.includes(fret) || fret === DOUBLE_INLAY_FRET) {
        parts.push(`<text x="${centreFor(fret)}" y="${height + 2}" class="fb-fretnum">${fret}</text>`);
      }
    }
    parts.push(`<rect x="${nutX - 4}" y="${top - 12}" width="5" height="${height - top * 2 + 24}" class="fb-nut"/>`);
    board.forEach((_, stringIndex) => {
      parts.push(`<line x1="${nutX - 26}" x2="${width - 12}" y1="${yFor(stringIndex)}" y2="${yFor(stringIndex)}" class="fb-string" style="stroke-width:${0.8 + (5 - stringIndex) * 0.28}"/>`);
    });
    // Out-of-range shading so the chosen position stands out.
    if (minFret > 0) parts.push(`<rect x="${nutX - 30}" y="0" width="${xFor(minFret - 1) - nutX + 30}" height="${height}" class="fb-shade"/>`);
    if (maxFret < NECK_FRET_COUNT) parts.push(`<rect x="${xFor(maxFret)}" y="0" width="${width - xFor(maxFret)}" height="${height}" class="fb-shade"/>`);

    board.forEach((string, stringIndex) => {
      string.forEach((cell, fret) => {
        if (!cell.inScale) return;
        const isInRange = fret >= minFret && fret <= maxFret;
        const isHighlighted = quiz?.highlight && quiz.highlight.stringIndex === stringIndex && quiz.highlight.fret === fret;
        const classes = ["fb-dot", cell.isRoot ? "root" : "", colourDegrees.has(cell.degree) ? "colour" : "", isInRange ? "" : "dim", isHighlighted ? "quiz" : ""];
        const label = isHidingLabels ? (isHighlighted ? "?" : "") : labels === "degrees" ? cell.degree : cell.name;
        parts.push(`<g class="${classes.join(" ")}" transform="translate(${centreFor(fret).toFixed(1)} ${yFor(stringIndex)})">
          <circle r="${isHighlighted ? 13 : 9.5}"/><text dy="3.6">${escapeHtml(label)}</text></g>`);
      });
    });

    $("fretboardSvg").innerHTML = `<svg ${SVG_NS_ATTR} viewBox="0 0 ${width} ${height + 8}" role="img" aria-label="${escapeHtml(`${rootName} ${scale.label} on the fretboard`)}">${parts.join("")}</svg>`;

    const notes = fretboard.scaleNotes(rootName, scaleId);
    $("scaleFormula").innerHTML = notes
      .map((note) => `<span class="pr-degree ${colourDegrees.has(note.degree) ? "colour" : ""} ${note.degree === "1" ? "root" : ""}"><b>${escapeHtml(note.degree)}</b>${escapeHtml(note.name)}</span>`)
      .join("");
  }

  function startQuiz() {
    ui.takeStartedAt = Date.now();
    ui.quiz = { asked: 0, correct: 0, question: null, answered: false };
    nextQuestion();
  }

  function endQuiz(shouldRecord) {
    const quiz = ui.quiz;
    ui.quiz = null;
    if (shouldRecord && quiz && quiz.asked > 0) {
      recordSession("fretboard", {
        accuracy: Math.round((100 * quiz.correct) / quiz.asked),
        questions: quiz.asked,
        correct: quiz.correct,
        scale: `${ui.board.rootName} ${fretboard.SCALES[ui.board.scaleId].label}`,
      });
    }
    renderQuiz(quiz && shouldRecord ? `round done: ${quiz.correct} / ${quiz.asked}` : "");
    renderFretboard();
  }

  function nextQuestion() {
    const quiz = ui.quiz;
    if (!quiz) return;
    if (quiz.asked >= QUIZ_ROUND_LENGTH) {
      endQuiz(true);
      return;
    }
    quiz.question = fretboard.createQuestion({ ...ui.board });
    quiz.answered = false;
    renderQuiz();
    renderFretboard();
  }

  function answerQuestion(choiceId) {
    const quiz = ui.quiz;
    if (!quiz || quiz.answered) return;
    const { isCorrect } = fretboard.gradeAnswer(quiz.question, choiceId);
    quiz.answered = true;
    quiz.asked += 1;
    if (isCorrect) quiz.correct += 1;
    renderQuiz("", choiceId);
  }

  function renderQuiz(doneMessage = "", chosenId = null) {
    const panel = $("quizPanel");
    if (!panel) return;
    const quiz = ui.quiz;
    $("btnQuiz").querySelector(".take-label").textContent = quiz ? "end quiz" : "quiz me";
    $("btnQuiz").classList.toggle("recording", Boolean(quiz));

    if (!quiz) {
      panel.innerHTML = doneMessage ? `<p class="pr-quiz-done">${escapeHtml(doneMessage)}</p>` : "";
      return;
    }
    const { question, answered } = quiz;
    const choices = question.choices
      .map((choice) => {
        const state = !answered ? "" : choice.id === question.answer ? "right" : choice.id === chosenId ? "wrong" : "";
        return `<button type="button" class="pr-choice ${state}" data-choice-id="${escapeHtml(choice.id)}" ${answered ? "disabled" : ""}>${escapeHtml(choice.label)}</button>`;
      })
      .join("");
    panel.innerHTML = `
      <p class="pr-quiz-progress">${quiz.asked + (answered ? 0 : 1)} / ${QUIZ_ROUND_LENGTH} · ${quiz.correct} right</p>
      <p class="pr-quiz-prompt">${escapeHtml(question.prompt)}</p>
      <div class="pr-choices">${choices}</div>
      ${answered ? `<button type="button" class="pr-chip" data-action="next-question">next</button>` : ""}`;
  }

  // ---- log ------------------------------------------------------------------------------------

  function renderLog() {
    if (!$("logStreak")) return;
    const now = new Date();
    const totals = practiceLog.totals(ui.log, { days: 7, now });
    setText("logStreak", String(practiceLog.currentStreakDays(ui.log, now)));
    setText("logWeekSessions", String(totals.sessionCount));
    setText("logWeekMinutes", String(totals.minutes));

    $("trendTiming").innerHTML = trendSvg(practiceLog.trend(ui.log, "timing", "score", { days: TREND_DAYS, now }), "timing score");
    $("trendVibrato").innerHTML = trendSvg(practiceLog.trend(ui.log, "vibrato", "score", { days: TREND_DAYS, now }), "vibrato score");
    $("trendFretboard").innerHTML = trendSvg(practiceLog.trend(ui.log, "fretboard", "accuracy", { days: TREND_DAYS, now }), "quiz accuracy");

    const recent = [...ui.log.sessions].reverse().slice(0, RECENT_SESSION_COUNT);
    $("logSessions").innerHTML = recent.length
      ? recent.map(sessionRowHtml).join("")
      : `<p class="pr-empty">sessions you finish in timing, vibrato and fretboard show up here</p>`;
  }

  function trendSvg(points, label) {
    const width = 280;
    const height = 80;
    if (points.length === 0) return `<p class="pr-empty small">no ${escapeHtml(label)} yet</p>`;
    const x = (index) => (points.length === 1 ? width / 2 : 8 + (index / (points.length - 1)) * (width - 16));
    const y = (value) => height - 8 - (Math.max(0, Math.min(100, value)) / 100) * (height - 16);
    const path = points.map((point, index) => `${index ? "L" : "M"}${x(index).toFixed(1)} ${y(point.value).toFixed(1)}`).join("");
    const dots = points
      .map((point, index) => `<circle cx="${x(index).toFixed(1)}" cy="${y(point.value).toFixed(1)}" r="3"><title>${point.day}: ${Math.round(point.value)}</title></circle>`)
      .join("");
    const latest = Math.round(points[points.length - 1].value);
    return `<svg ${SVG_NS_ATTR} viewBox="0 0 ${width} ${height}" role="img" aria-label="${escapeHtml(label)} over the last ${TREND_DAYS} days, latest ${latest}">
      <path d="${path}" class="pr-trend"/>${dots}</svg><span class="pr-trend-latest">${latest}</span>`;
  }

  function sessionRowHtml(session) {
    const date = new Date(session.startedAt);
    const when = `${date.toLocaleDateString(undefined, { month: "short", day: "numeric" })} ${date.toLocaleTimeString(undefined, { hour: "2-digit", minute: "2-digit" })}`;
    const metrics = session.metrics;
    const detail =
      session.kind === "timing"
        ? `${metrics.score} · ${metrics.bpm} bpm · ${formatSigned(metrics.meanDeviationMs ?? 0)} ms ${metrics.tendency ?? ""}`
        : session.kind === "vibrato"
          ? `${metrics.score} · ${metrics.rateHz} Hz · ${metrics.widthCents}¢ · even ${metrics.consistency}`
          : `${metrics.accuracy}% · ${escapeHtml(metrics.scale ?? "")}`;
    return `<div class="pr-session" data-kind="${session.kind}">
      <span class="pr-session-kind">${session.kind}</span>
      <span class="pr-session-detail">${detail}</span>
      <span class="pr-session-when">${escapeHtml(when)}</span>
      <button type="button" class="pr-session-delete" data-delete-session="${escapeHtml(session.id)}" aria-label="Delete session">×</button>
    </div>`;
  }

  // ---- events from the page ---------------------------------------------------------------------

  function bindEvents() {
    const scene = $("practiceScene");
    if (!scene) return;

    scene.addEventListener("click", (event) => {
      const target = event.target.closest("button");
      if (!target || !scene.contains(target)) return;
      const { dataset } = target;

      if (dataset.practiceTool) showTool(dataset.practiceTool);
      else if (dataset.take) toggleTake(dataset.take);
      else if (dataset.subdivision) {
        updateLog(practiceLog.setTargets(ui.log, "timing", { subdivision: Number(dataset.subdivision) }));
        reanalyseLastTimingTake();
      } else if (dataset.tolerance) {
        updateLog(practiceLog.setTargets(ui.log, "timing", { toleranceMs: Number(dataset.tolerance) }));
        reanalyseLastTimingTake();
      }
      else if (dataset.bpmStep) {
        bridge.setTempo((bridge.getParameter("metronomeBpm") ?? 120) + Number(dataset.bpmStep));
        setText("practiceBpm", Math.round(bridge.getParameter("metronomeBpm")));
      } else if (dataset.vibratoTarget) nudgeVibratoTarget(dataset.vibratoTarget, Number(dataset.step));
      else if (dataset.action === "reset-latency") updateLog(practiceLog.setSettings(ui.log, { latencyOffsetMs: 0 }));
      else if (dataset.boardLabels) {
        ui.board.labels = dataset.boardLabels;
        renderFretboardControls();
        renderFretboard();
      } else if (dataset.action === "quiz") (ui.quiz ? endQuiz(true) : startQuiz());
      else if (dataset.choiceId) answerQuestion(dataset.choiceId);
      else if (dataset.action === "next-question") nextQuestion();
      else if (dataset.deleteSession) updateLog(practiceLog.deleteSession(ui.log, dataset.deleteSession));
    });

    const onBoardChange = () => {
      const minFret = Number($("boardMinFret").value);
      const maxFret = Number($("boardMaxFret").value);
      ui.board = {
        ...ui.board,
        rootName: $("boardRoot").value,
        scaleId: $("boardScale").value,
        tuningId: $("boardTuning").value,
        minFret: Math.min(minFret, maxFret),
        maxFret: Math.max(minFret, maxFret),
      };
      if (ui.quiz) endQuiz(false);
      renderFretboard();
    };
    ["boardRoot", "boardScale", "boardTuning", "boardMinFret", "boardMaxFret"].forEach((id) => $(id)?.addEventListener("change", onBoardChange));
  }

  // ---- public API -----------------------------------------------------------------------------------

  // bridge: { send(message), getParameter(id), setTempo(bpm), setMetronomeEnabled(on) }
  function init(appBridge) {
    bridge = appBridge;
    bindEvents();
    if (typeof ResizeObserver === "function" && $("timingChart")) new ResizeObserver(paintTimingChart).observe($("timingChart"));
    renderFretboardControls();
    renderTargets();
    renderTimingResult(false);
    renderVibratoResult();
    renderQuiz();
    showTool(ui.tool);
    bridge.send({ type: "practiceCapture", enabled: false });
  }

  function receive(message) {
    if (message.type === "practiceLog") {
      ui.log = practiceLog.normaliseLog(message.log);
      renderTargets();
      renderLog();
    } else if (message.type === "practiceCaptureState") {
      ui.capture = {
        enabled: Boolean(message.enabled),
        sampleRate: Number(message.sampleRate) || ui.capture.sampleRate,
        deviceLatencySamples: Number(message.deviceLatencySamples) || 0,
      };
      renderLatency();
      if (!ui.capture.enabled && ui.isStopping) finishTake();
    } else if (message.type === "practiceEvents") {
      onEventsBatch(message);
    } else if (message.type === "practice") {
      const beat = Number(message.beatInBar);
      if (beat !== ui.beatInBar) {
        ui.beatInBar = beat;
        const beatsPerBar = Number(message.beatsPerBar) || 4;
        document.querySelectorAll(".pr-beat").forEach((light, index) => {
          // The engine advances beatInBar as each click fires, so the one that just sounded is one behind.
          const sounded = (beat + beatsPerBar - 1) % beatsPerBar;
          light.classList.toggle("on", Boolean(message.metronomeEnabled) && index === sounded);
        });
      }
      setText("practiceBpm", Math.round(Number(message.metronomeBpm) || 0));
    }
  }

  // Called when the user leaves the Practice section, so a take never runs unseen.
  function onHidden() {
    if (isCapturing()) stopTake();
  }

  practice.ui = { init, receive, onHidden };
})(typeof self !== "undefined" ? self : this);
