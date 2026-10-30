// Vibrato analysis from the tuner's pitch stream.
//
// For each sustained note it reports:
//   rateHz      oscillations per second
//   widthCents  peak-to-peak pitch swing (100 cents = one semitone)
//   consistency how even the cycles are (0-100), the thing that separates a controlled
//               vocal-style vibrato from a nervous wobble
//
// Slow pitch movement (a bend, a slide, drifting intonation) is removed with a moving
// average before measuring, so bend-and-vibrato notes are measured on the vibrato alone.
(function attach(root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) module.exports = api;
  else root.LumenPractice = Object.assign(root.LumenPractice || {}, { vibrato: api });
})(typeof self !== "undefined" ? self : this, function createVibratoModule() {
  "use strict";

  const CENTS_PER_OCTAVE = 1200;
  const CENTS_PER_SEMITONE = 100;
  const REFERENCE_HZ = 440;
  const REFERENCE_MIDI = 69;
  const MILLISECONDS_PER_SECOND = 1000;
  const NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];

  const DEFAULTS = Object.freeze({
    // A guitar vibrato cycle changes pitch by ~20 cents per tuner frame at most;
    // a bigger jump between frames means a new note.
    noteChangeCents: 80,
    // Short gaps (a frame or two where the tuner lost lock) do not split a note.
    maxGapMs: 30,
    // Vibrato needs time to show itself: two cycles at 4 Hz is 500 ms.
    minimumNoteMs: 450,
    // The pick attack is pitch-unstable; skip it.
    attackSkipMs: 60,
    // Window for removing slow drift. Roughly one cycle of the slowest vibrato we care
    // about (3.3 Hz), which averages a full cycle to ~zero and leaves it in the residual.
    detrendWindowMs: 300,
    // Hysteresis on the zero-crossing detector so tiny jitter is not counted as cycles.
    crossingHysteresisCents: 3,
    // Below this peak-to-peak swing a note counts as straight (no vibrato).
    minimumWidthCents: 12,
    minimumCycles: 2,
  });

  function hzToCents(frequencyHz) {
    return CENTS_PER_OCTAVE * Math.log2(frequencyHz / REFERENCE_HZ);
  }

  function noteNameForCents(cents) {
    const midi = Math.round(REFERENCE_MIDI + cents / CENTS_PER_SEMITONE);
    const pitchClass = ((midi % 12) + 12) % 12;
    return `${NOTE_NAMES[pitchClass]}${Math.floor(midi / 12) - 1}`;
  }

  function mean(values) {
    return values.length ? values.reduce((sum, value) => sum + value, 0) / values.length : 0;
  }

  function median(values) {
    if (!values.length) return 0;
    const sorted = [...values].sort((a, b) => a - b);
    const middle = Math.floor(sorted.length / 2);
    return sorted.length % 2 ? sorted[middle] : (sorted[middle - 1] + sorted[middle]) / 2;
  }

  // Coefficient of variation mapped to 0-100, where 100 means perfectly even.
  // 25% variation or more scores 0.
  function evenness(values) {
    if (values.length < 2) return 0;
    const average = mean(values);
    if (average === 0) return 0;
    const deviation = Math.sqrt(mean(values.map((value) => (value - average) ** 2)));
    const maxUsefulVariation = 0.25;
    return Math.round(100 * Math.max(0, 1 - deviation / average / maxUsefulVariation));
  }

  // Splits the frame stream into sustained notes: voiced runs with no big pitch jumps.
  function segmentNotes(frames, settings) {
    const notes = [];
    let current = [];

    const flush = () => {
      if (current.length) notes.push(current);
      current = [];
    };

    for (const frame of frames) {
      const isVoiced = frame.frequencyHz > 0 && Number.isFinite(frame.frequencyHz);
      if (!isVoiced) {
        const last = current[current.length - 1];
        if (last && frame.timeMs - last.timeMs > settings.maxGapMs) flush();
        continue;
      }

      const point = { timeMs: frame.timeMs, cents: hzToCents(frame.frequencyHz) };
      const last = current[current.length - 1];
      const isGapTooLong = last && point.timeMs - last.timeMs > settings.maxGapMs;
      const isNewPitch = last && Math.abs(point.cents - last.cents) > settings.noteChangeCents;
      if (isGapTooLong || isNewPitch) flush();
      current.push(point);
    }
    flush();
    return notes;
  }

  // Centered moving average over a time window; frames are close to evenly spaced.
  function movingAverage(points, windowMs) {
    const halfWindow = windowMs / 2;
    let start = 0;
    let end = 0;
    let sum = 0;
    return points.map((point) => {
      while (end < points.length && points[end].timeMs <= point.timeMs + halfWindow) sum += points[end++].cents;
      while (points[start].timeMs < point.timeMs - halfWindow) sum -= points[start++].cents;
      return sum / (end - start);
    });
  }

  // Indices of upward and downward zero crossings, with hysteresis.
  function findCrossings(residual, hysteresis) {
    const crossings = [];
    let side = 0; // -1 below, +1 above, 0 unknown
    residual.forEach((value, index) => {
      if (value > hysteresis && side !== 1) {
        if (side === -1) crossings.push({ index, direction: 1 });
        side = 1;
      } else if (value < -hysteresis && side !== -1) {
        if (side === 1) crossings.push({ index, direction: -1 });
        side = -1;
      }
    });
    return crossings;
  }

  function detrend(points, windowMs) {
    const trend = movingAverage(points, windowMs);
    return points.map((point, index) => point.cents - trend[index]);
  }

  function medianCyclePeriodMs(points, residual, hysteresis) {
    const upward = findCrossings(residual, hysteresis).filter((crossing) => crossing.direction === 1);
    const periods = [];
    for (let cycle = 1; cycle < upward.length; cycle += 1) {
      periods.push(points[upward[cycle].index].timeMs - points[upward[cycle - 1].index].timeMs);
    }
    return median(periods);
  }

  function straightNote(base) {
    return { ...base, hasVibrato: false, rateHz: 0, widthCents: 0, cycles: 0, rateConsistency: 0, widthConsistency: 0, consistency: 0 };
  }

  function analyseNote(points, settings) {
    const start = points[0].timeMs;
    const body = points.filter((point) => point.timeMs >= start + settings.attackSkipMs);
    const durationMs = points[points.length - 1].timeMs - start;
    if (body.length < 4) return null;

    // Two-pass detrend. A moving average only cancels a vibrato exactly when its window is
    // a whole number of cycles; otherwise part of the cycle leaks back into the residual and
    // inflates the width (by ~17% at 5.5 Hz with a 300 ms window). So estimate the period
    // with a generic window first, then detrend again over exactly one estimated cycle.
    const roughResidual = detrend(body, settings.detrendWindowMs);
    const roughPeriodMs = medianCyclePeriodMs(body, roughResidual, settings.crossingHysteresisCents);
    const residual = roughPeriodMs > 0 ? detrend(body, roughPeriodMs) : roughResidual;
    const centreCents = median(body.map((point) => point.cents));
    const base = {
      startMs: start,
      durationMs,
      noteName: noteNameForCents(centreCents),
      // How far the held pitch sits from equal temperament: intonation, not vibrato.
      intonationCents: centreCents - CENTS_PER_SEMITONE * Math.round(centreCents / CENTS_PER_SEMITONE),
    };

    // Measure full cycles between consecutive upward crossings.
    const upward = findCrossings(residual, settings.crossingHysteresisCents).filter((crossing) => crossing.direction === 1);
    const periodsMs = [];
    const widthsCents = [];
    for (let cycle = 1; cycle < upward.length; cycle += 1) {
      const from = upward[cycle - 1].index;
      const to = upward[cycle].index;
      periodsMs.push(body[to].timeMs - body[from].timeMs);
      const slice = residual.slice(from, to + 1);
      widthsCents.push(Math.max(...slice) - Math.min(...slice));
    }

    if (periodsMs.length < settings.minimumCycles) return straightNote(base);

    const widthCents = median(widthsCents);
    if (widthCents < settings.minimumWidthCents) return straightNote(base);

    const rateConsistency = evenness(periodsMs);
    const widthConsistency = evenness(widthsCents);
    return {
      ...base,
      hasVibrato: true,
      rateHz: MILLISECONDS_PER_SECOND / median(periodsMs),
      widthCents,
      cycles: periodsMs.length,
      rateConsistency,
      widthConsistency,
      consistency: Math.round((rateConsistency + widthConsistency) / 2),
      // The detrended curve, for drawing. Downsampled points keep the bridge payload small.
      curve: body.map((point, index) => ({ timeMs: point.timeMs - start, cents: residual[index] })),
    };
  }

  // frames: [{ timeMs, frequencyHz }] in time order (frequencyHz 0 = unvoiced).
  function analyseVibrato(frames, overrides) {
    const settings = { ...DEFAULTS, ...overrides };
    if (!Array.isArray(frames) || frames.length === 0) {
      return { notes: [], vibratoNotes: [], summary: null };
    }

    const notes = segmentNotes(frames, settings)
      .filter((points) => points[points.length - 1].timeMs - points[0].timeMs >= settings.minimumNoteMs)
      .map((points) => analyseNote(points, settings))
      .filter(Boolean);

    const vibratoNotes = notes.filter((note) => note.hasVibrato);
    const summary = vibratoNotes.length
      ? {
          noteCount: vibratoNotes.length,
          rateHz: median(vibratoNotes.map((note) => note.rateHz)),
          widthCents: median(vibratoNotes.map((note) => note.widthCents)),
          consistency: Math.round(mean(vibratoNotes.map((note) => note.consistency))),
        }
      : null;

    return { notes, vibratoNotes, summary };
  }

  // How close a measured vibrato is to the player's own target (0-100).
  // Rate and width are compared on a ratio (log) scale so "too fast" and "too slow" are
  // penalised symmetrically; 1.5x off in either direction scores 0 on that component.
  function scoreAgainstTarget(summary, target) {
    if (!summary || !target || !(target.rateHz > 0) || !(target.widthCents > 0)) return 0;
    const closeness = (actual, wanted) => Math.max(0, 1 - Math.abs(Math.log(actual / wanted)) / Math.log(1.5));
    const rate = closeness(summary.rateHz, target.rateHz);
    const width = closeness(summary.widthCents, target.widthCents);
    const consistency = summary.consistency / 100;
    return Math.round(100 * (rate * 0.35 + width * 0.35 + consistency * 0.3));
  }

  return { DEFAULTS, analyseVibrato, scoreAgainstTarget, hzToCents, noteNameForCents, segmentNotes };
});
