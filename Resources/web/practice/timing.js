// Timing analysis: how far each pick lands from the metronome grid.
//
// Everything is measured on the engine's sample clock. The click is generated at the
// output and the player reacts to what they HEAR, then their pick travels back through
// the input, so every onset arrives late by the device round-trip latency. That latency
// (plus any personal offset found by calibration) is subtracted before scoring.
(function attach(root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) module.exports = api;
  else root.LumenPractice = Object.assign(root.LumenPractice || {}, { timing: api });
})(typeof self !== "undefined" ? self : this, function createTimingModule() {
  "use strict";

  const MILLISECONDS_PER_SECOND = 1000;
  const SUPPORTED_SUBDIVISIONS = Object.freeze([1, 2, 3, 4, 6, 8]);
  const DEFAULT_TOLERANCE_MS = 15;
  // A mean offset smaller than this is within the noise of the detector and the ear.
  const TENDENCY_THRESHOLD_MS = 5;
  // RMS error at which the score bottoms out. 30 ms is clearly audible as sloppy.
  const SCORE_ZERO_RMS_MS = 30;
  const MINIMUM_BEATS_FOR_GRID = 2;
  const MINIMUM_HITS_FOR_CALIBRATION = 4;

  function samplesToMs(samples, sampleRate) {
    return (samples / sampleRate) * MILLISECONDS_PER_SECOND;
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

  function standardDeviation(values) {
    if (values.length < 2) return 0;
    const average = mean(values);
    return Math.sqrt(mean(values.map((value) => (value - average) ** 2)));
  }

  function rootMeanSquare(values) {
    return values.length ? Math.sqrt(mean(values.map((value) => value * value))) : 0;
  }

  // Grid slots (sample positions) built from the actual click times rather than a nominal
  // BPM, so tempo changes mid-take and audio-clock drift are handled for free.
  // The last beat is extended by one interval so notes played after the final click still score.
  function buildGrid(beats, subdivision) {
    const sorted = [...beats].sort((a, b) => a.sampleIndex - b.sampleIndex);
    const slots = [];
    for (let beat = 0; beat < sorted.length; beat += 1) {
      const start = sorted[beat].sampleIndex;
      const next = sorted[beat + 1]?.sampleIndex ?? start + (start - sorted[beat - 1].sampleIndex);
      const slotLength = (next - start) / subdivision;
      for (let slot = 0; slot < subdivision; slot += 1) {
        slots.push({ sampleIndex: start + slot * slotLength, slotInBeat: slot, beatInBar: sorted[beat].beatInBar, slotLength });
      }
    }
    return slots;
  }

  function nearestSlotIndex(slots, sampleIndex) {
    let low = 0;
    let high = slots.length - 1;
    while (low < high) {
      const middle = (low + high) >> 1;
      if (slots[middle].sampleIndex < sampleIndex) low = middle + 1;
      else high = middle;
    }
    if (low > 0 && Math.abs(slots[low - 1].sampleIndex - sampleIndex) <= Math.abs(slots[low].sampleIndex - sampleIndex)) {
      return low - 1;
    }
    return low;
  }

  function describeTendency(meanDeviationMs) {
    if (meanDeviationMs <= -TENDENCY_THRESHOLD_MS) return "rushing";
    if (meanDeviationMs >= TENDENCY_THRESHOLD_MS) return "dragging";
    return "on-time";
  }

  function emptyAnalysis(reason) {
    return {
      isValid: false,
      reason,
      hits: [],
      extraOnsetCount: 0,
      outOfRangeCount: 0,
      meanDeviationMs: 0,
      medianDeviationMs: 0,
      standardDeviationMs: 0,
      rmsDeviationMs: 0,
      tightPercent: 0,
      tendency: "on-time",
      bySlotInBeat: [],
      score: 0,
    };
  }

  // Positive deviation = late (dragging), negative = early (rushing).
  //
  // options:
  //   sampleRate     (required) engine sample rate
  //   subdivision    notes per click: 1 quarters, 2 eighths, 3 triplets, 4 sixteenths, ...
  //   latencyMs      round-trip latency + personal offset to subtract from every onset
  //   toleranceMs    |deviation| at or under this counts as "tight"
  function analyseTiming(beats, onsets, options) {
    const { sampleRate, subdivision = 1, latencyMs = 0, toleranceMs = DEFAULT_TOLERANCE_MS } = options || {};

    if (!(sampleRate > 0)) return emptyAnalysis("Missing sample rate");
    if (!SUPPORTED_SUBDIVISIONS.includes(subdivision)) return emptyAnalysis(`Unsupported subdivision ${subdivision}`);
    if (!Array.isArray(beats) || beats.length < MINIMUM_BEATS_FOR_GRID) {
      return emptyAnalysis("Need at least two metronome clicks; is the metronome running?");
    }
    if (!Array.isArray(onsets) || onsets.length === 0) return emptyAnalysis("No picked notes detected");

    const slots = buildGrid(beats, subdivision);
    const latencySamples = (latencyMs / MILLISECONDS_PER_SECOND) * sampleRate;
    const firstUsable = slots[0].sampleIndex - slots[0].slotLength / 2;
    const lastUsable = slots[slots.length - 1].sampleIndex + slots[slots.length - 1].slotLength / 2;

    // Each grid slot can be claimed by one onset: the closest. Extra onsets near the same slot
    // (a double-pick, a ghost note) are counted separately instead of skewing the average.
    const bestBySlot = new Map();
    let outOfRangeCount = 0;

    for (const onset of onsets) {
      const corrected = onset.sampleIndex - latencySamples;
      if (corrected < firstUsable || corrected > lastUsable) {
        outOfRangeCount += 1;
        continue;
      }
      const slotIndex = nearestSlotIndex(slots, corrected);
      const deviationSamples = corrected - slots[slotIndex].sampleIndex;
      const existing = bestBySlot.get(slotIndex);
      if (!existing || Math.abs(deviationSamples) < Math.abs(existing.deviationSamples)) {
        bestBySlot.set(slotIndex, { slotIndex, deviationSamples });
      }
    }

    const claimedCount = bestBySlot.size;
    const inRangeCount = onsets.length - outOfRangeCount;
    if (claimedCount === 0) return { ...emptyAnalysis("No picked notes landed inside the click grid"), outOfRangeCount };

    const hits = [...bestBySlot.values()]
      .sort((a, b) => a.slotIndex - b.slotIndex)
      .map(({ slotIndex, deviationSamples }) => ({
        slotIndex,
        slotInBeat: slots[slotIndex].slotInBeat,
        beatInBar: slots[slotIndex].beatInBar,
        timeMs: samplesToMs(slots[slotIndex].sampleIndex - slots[0].sampleIndex, sampleRate),
        deviationMs: samplesToMs(deviationSamples, sampleRate),
      }));

    const deviations = hits.map((hit) => hit.deviationMs);
    const meanDeviationMs = mean(deviations);
    const rmsDeviationMs = rootMeanSquare(deviations);

    const bySlotInBeat = Array.from({ length: subdivision }, (_, slotInBeat) => {
      const values = hits.filter((hit) => hit.slotInBeat === slotInBeat).map((hit) => hit.deviationMs);
      return { slotInBeat, count: values.length, meanDeviationMs: mean(values) };
    });

    return {
      isValid: true,
      reason: "",
      hits,
      extraOnsetCount: inRangeCount - claimedCount,
      outOfRangeCount,
      meanDeviationMs,
      medianDeviationMs: median(deviations),
      standardDeviationMs: standardDeviation(deviations),
      rmsDeviationMs,
      tightPercent: (100 * deviations.filter((value) => Math.abs(value) <= toleranceMs).length) / deviations.length,
      tendency: describeTendency(meanDeviationMs),
      bySlotInBeat,
      score: Math.round(100 * Math.max(0, 1 - rmsDeviationMs / SCORE_ZERO_RMS_MS)),
    };
  }

  // Calibration: the player picks along with quarter-note clicks; the median offset is
  // the latency the device report missed plus their own habitual push or lay-back.
  // Median, not mean, so one flubbed note does not skew it.
  function estimateLatencyMs(beats, onsets, options) {
    const { sampleRate, baseLatencyMs = 0 } = options || {};
    const analysis = analyseTiming(beats, onsets, { sampleRate, subdivision: 1, latencyMs: baseLatencyMs });
    if (!analysis.isValid || analysis.hits.length < MINIMUM_HITS_FOR_CALIBRATION) {
      return { isValid: false, latencyMs: baseLatencyMs, hitCount: analysis.hits.length };
    }
    return {
      isValid: true,
      latencyMs: baseLatencyMs + analysis.medianDeviationMs,
      hitCount: analysis.hits.length,
      spreadMs: analysis.standardDeviationMs,
    };
  }

  return {
    SUPPORTED_SUBDIVISIONS,
    DEFAULT_TOLERANCE_MS,
    MINIMUM_HITS_FOR_CALIBRATION,
    analyseTiming,
    estimateLatencyMs,
    buildGrid,
    describeTendency,
  };
});
