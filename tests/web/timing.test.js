const test = require("node:test");
const assert = require("node:assert/strict");
const { analyseTiming, estimateLatencyMs, buildGrid } = require("../../Resources/web/practice/timing.js");

const SAMPLE_RATE = 48000;
const msToSamples = (milliseconds) => (milliseconds / 1000) * SAMPLE_RATE;

function clicks(bpm, count, startMs = 1000) {
  const intervalMs = 60000 / bpm;
  return Array.from({ length: count }, (_, index) => ({
    sampleIndex: msToSamples(startMs + index * intervalMs),
    beatInBar: index % 4,
  }));
}

// Places one onset on every grid slot, shifted by offsetMs (positive = late).
function onsetsOnGrid(beats, subdivision, offsetMs = 0) {
  return buildGrid(beats, subdivision)
    .slice(0, (beats.length - 1) * subdivision)
    .map((slot) => ({ sampleIndex: slot.sampleIndex + msToSamples(offsetMs), strength: 5 }));
}

test("perfect quarter notes score 100 and read as on-time", () => {
  const beats = clicks(100, 9);
  const result = analyseTiming(beats, onsetsOnGrid(beats, 1), { sampleRate: SAMPLE_RATE });
  assert.equal(result.isValid, true);
  assert.equal(result.hits.length, 8);
  assert.ok(Math.abs(result.meanDeviationMs) < 1e-9);
  assert.equal(result.score, 100);
  assert.equal(result.tightPercent, 100);
  assert.equal(result.tendency, "on-time");
});

test("consistently late picks read as dragging with the right offset", () => {
  const beats = clicks(120, 9);
  const result = analyseTiming(beats, onsetsOnGrid(beats, 1, 12), { sampleRate: SAMPLE_RATE });
  assert.equal(result.tendency, "dragging");
  assert.ok(Math.abs(result.meanDeviationMs - 12) < 0.01);
  assert.ok(result.score < 100);
});

test("consistently early picks read as rushing", () => {
  const beats = clicks(120, 9);
  const result = analyseTiming(beats, onsetsOnGrid(beats, 2, -10), { sampleRate: SAMPLE_RATE });
  assert.equal(result.tendency, "rushing");
  assert.ok(Math.abs(result.meanDeviationMs + 10) < 0.01);
});

test("latency compensation removes the round trip delay", () => {
  const beats = clicks(90, 9);
  const delayed = onsetsOnGrid(beats, 1, 25);
  const result = analyseTiming(beats, delayed, { sampleRate: SAMPLE_RATE, latencyMs: 25 });
  assert.ok(Math.abs(result.meanDeviationMs) < 0.01);
  assert.equal(result.score, 100);
});

test("sixteenths are scored per slot in the beat", () => {
  const beats = clicks(110, 5);
  const result = analyseTiming(beats, onsetsOnGrid(beats, 4), { sampleRate: SAMPLE_RATE, subdivision: 4 });
  assert.equal(result.hits.length, 16);
  assert.deepEqual(result.bySlotInBeat.map((slot) => slot.count), [4, 4, 4, 4]);
});

test("a late 'e' and 'a' shows up in the per-slot breakdown", () => {
  // A common habit: downbeats locked, off-beat 16ths swung late.
  const beats = clicks(100, 5);
  const onsets = buildGrid(beats, 4)
    .slice(0, 16)
    .map((slot) => ({ sampleIndex: slot.sampleIndex + msToSamples(slot.slotInBeat % 2 ? 15 : 0) }));
  const result = analyseTiming(beats, onsets, { sampleRate: SAMPLE_RATE, subdivision: 4 });
  assert.ok(Math.abs(result.bySlotInBeat[0].meanDeviationMs) < 0.01);
  assert.ok(Math.abs(result.bySlotInBeat[1].meanDeviationMs - 15) < 0.01);
  assert.ok(Math.abs(result.bySlotInBeat[3].meanDeviationMs - 15) < 0.01);
});

test("triplets use a three-slot grid", () => {
  const beats = clicks(80, 4);
  const result = analyseTiming(beats, onsetsOnGrid(beats, 3), { sampleRate: SAMPLE_RATE, subdivision: 3 });
  assert.equal(result.hits.length, 9);
  assert.equal(result.score, 100);
});

test("grid follows the real clicks when tempo changes mid-take", () => {
  const slow = clicks(90, 5, 1000);
  const lastSlowMs = 1000 + 4 * (60000 / 90);
  const fast = clicks(140, 5, lastSlowMs + 60000 / 140);
  const beats = [...slow, ...fast];
  const result = analyseTiming(beats, onsetsOnGrid(beats, 2), { sampleRate: SAMPLE_RATE, subdivision: 2 });
  assert.equal(result.score, 100);
});

test("notes after the final click still score within one beat", () => {
  const beats = clicks(120, 4);
  const lastClick = beats[beats.length - 1].sampleIndex;
  const result = analyseTiming(beats, [{ sampleIndex: lastClick }, { sampleIndex: lastClick + msToSamples(250) }], {
    sampleRate: SAMPLE_RATE,
    subdivision: 2,
  });
  assert.equal(result.hits.length, 2);
  assert.equal(result.outOfRangeCount, 0);
});

test("a double pick near one slot is counted as extra, not averaged in", () => {
  const beats = clicks(100, 5);
  const onsets = onsetsOnGrid(beats, 1);
  onsets.push({ sampleIndex: onsets[1].sampleIndex + msToSamples(40) });
  const result = analyseTiming(beats, onsets, { sampleRate: SAMPLE_RATE });
  assert.equal(result.extraOnsetCount, 1);
  assert.equal(result.score, 100);
});

test("onsets before the grid starts are ignored and counted", () => {
  const beats = clicks(100, 5, 2000);
  const onsets = [{ sampleIndex: msToSamples(100) }, ...onsetsOnGrid(beats, 1)];
  const result = analyseTiming(beats, onsets, { sampleRate: SAMPLE_RATE });
  assert.equal(result.outOfRangeCount, 1);
  assert.equal(result.hits.length, 4);
});

test("unsorted click input gives the same answer", () => {
  const beats = clicks(100, 7);
  const onsets = onsetsOnGrid(beats, 1, 7);
  const sorted = analyseTiming(beats, onsets, { sampleRate: SAMPLE_RATE });
  const shuffled = analyseTiming([...beats].reverse(), onsets, { sampleRate: SAMPLE_RATE });
  assert.deepEqual(shuffled, sorted);
});

test("tolerance controls the tight percentage", () => {
  const beats = clicks(100, 5);
  const onsets = onsetsOnGrid(beats, 1).map((onset, index) => ({ sampleIndex: onset.sampleIndex + msToSamples(index * 10) }));
  const result = analyseTiming(beats, onsets, { sampleRate: SAMPLE_RATE, toleranceMs: 15 });
  // Deviations 0, 10, 20, 30 ms: two within 15 ms.
  assert.equal(result.tightPercent, 50);
});

test("invalid inputs return an explanation instead of throwing", () => {
  const beats = clicks(100, 5);
  const onsets = onsetsOnGrid(beats, 1);
  assert.match(analyseTiming([beats[0]], onsets, { sampleRate: SAMPLE_RATE }).reason, /two metronome clicks/);
  assert.match(analyseTiming(beats, [], { sampleRate: SAMPLE_RATE }).reason, /No picked notes/);
  assert.match(analyseTiming(beats, onsets, { sampleRate: SAMPLE_RATE, subdivision: 5 }).reason, /Unsupported/);
  assert.match(analyseTiming(beats, onsets, {}).reason, /sample rate/);
  assert.equal(analyseTiming(null, null, null).isValid, false);
});

test("onsets that all miss the grid are reported, not scored", () => {
  const beats = clicks(100, 3, 5000);
  const result = analyseTiming(beats, [{ sampleIndex: 10 }], { sampleRate: SAMPLE_RATE });
  assert.equal(result.isValid, false);
  assert.equal(result.outOfRangeCount, 1);
});

test("latency calibration finds the median offset", () => {
  const beats = clicks(90, 10);
  const onsets = onsetsOnGrid(beats, 1, 38);
  onsets[3] = { sampleIndex: onsets[3].sampleIndex + msToSamples(60) }; // one flub
  const result = estimateLatencyMs(beats, onsets, { sampleRate: SAMPLE_RATE });
  assert.equal(result.isValid, true);
  assert.ok(Math.abs(result.latencyMs - 38) < 0.01);
});

test("calibration adds to the device-reported latency", () => {
  const beats = clicks(90, 10);
  const onsets = onsetsOnGrid(beats, 1, 30);
  const result = estimateLatencyMs(beats, onsets, { sampleRate: SAMPLE_RATE, baseLatencyMs: 22 });
  assert.ok(Math.abs(result.latencyMs - 30) < 0.01);
});

test("calibration refuses with too few hits", () => {
  const beats = clicks(90, 3);
  const result = estimateLatencyMs(beats, onsetsOnGrid(beats, 1), { sampleRate: SAMPLE_RATE, baseLatencyMs: 12 });
  assert.equal(result.isValid, false);
  assert.equal(result.latencyMs, 12);
});
