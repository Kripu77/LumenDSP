// Parses the compact practice event batches the C++ engine streams during a capture
// and accumulates them into one take.
//
// Loaded as a classic <script> in the app (attaches to window.LumenPractice) and as a
// CommonJS module under `node --test`, so the analysis can be tested without a WebView.
(function attach(root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) module.exports = api;
  else root.LumenPractice = Object.assign(root.LumenPractice || {}, { events: api });
})(typeof self !== "undefined" ? self : this, function createEventsModule() {
  "use strict";

  // Must match lumen::practice::PracticeEventKind in Source/practice/PracticeEventQueue.h.
  const EVENT_KIND = Object.freeze({ beat: 0, onset: 1, pitch: 2 });
  const MILLISECONDS_PER_SECOND = 1000;

  function createTake() {
    return { sampleRate: 0, beats: [], onsets: [], pitchFrames: [], droppedEvents: 0, isInterrupted: false };
  }

  function isValidTuple(tuple) {
    return Array.isArray(tuple) && tuple.length >= 3 && tuple.slice(0, 3).every(Number.isFinite);
  }

  // Appends one `practiceEvents` batch to a take. Returns the same take for chaining.
  // Malformed tuples are skipped rather than aborting the whole take.
  function appendBatch(take, batch) {
    if (!batch || !Array.isArray(batch.events)) return take;

    const sampleRate = Number(batch.sampleRate);
    if (Number.isFinite(sampleRate) && sampleRate > 0) {
      // A sample-rate change mid-take (the audio device was switched) makes the event
      // times incomparable, so the take is flagged rather than silently mis-scored.
      if (take.sampleRate > 0 && sampleRate !== take.sampleRate) take.isInterrupted = true;
      take.sampleRate = sampleRate;
    }
    take.droppedEvents = Math.max(take.droppedEvents, Number(batch.dropped) || 0);

    for (const tuple of batch.events) {
      if (!isValidTuple(tuple)) continue;
      const [kind, sampleIndex, value, confidence = 0] = tuple;

      if (kind === EVENT_KIND.beat) take.beats.push({ sampleIndex, beatInBar: Math.round(value) });
      else if (kind === EVENT_KIND.onset) take.onsets.push({ sampleIndex, strength: value });
      else if (kind === EVENT_KIND.pitch) take.pitchFrames.push({ sampleIndex, frequencyHz: value, confidence });
    }
    return take;
  }

  function samplesToMs(sampleIndex, sampleRate) {
    return (sampleIndex / sampleRate) * MILLISECONDS_PER_SECOND;
  }

  function msToSamples(milliseconds, sampleRate) {
    return (milliseconds / MILLISECONDS_PER_SECOND) * sampleRate;
  }

  // Pitch frames in milliseconds relative to the first frame, the shape the vibrato analyser wants.
  function pitchFramesInMs(take) {
    if (!take.sampleRate || take.pitchFrames.length === 0) return [];
    const origin = take.pitchFrames[0].sampleIndex;
    return take.pitchFrames.map((frame) => ({
      timeMs: samplesToMs(frame.sampleIndex - origin, take.sampleRate),
      frequencyHz: frame.frequencyHz,
    }));
  }

  return { EVENT_KIND, createTake, appendBatch, samplesToMs, msToSamples, pitchFramesInMs };
});
