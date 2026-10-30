const test = require("node:test");
const assert = require("node:assert/strict");
const { EVENT_KIND, createTake, appendBatch, pitchFramesInMs } = require("../../Resources/web/practice/events.js");

test("splits a batch into beats, onsets and pitch frames", () => {
  const take = appendBatch(createTake(), {
    sampleRate: 48000,
    dropped: 0,
    events: [
      [EVENT_KIND.beat, 1000, 0, 1],
      [EVENT_KIND.onset, 1200, 6.5, 1],
      [EVENT_KIND.pitch, 1300, 196, 0.9],
    ],
  });
  assert.equal(take.sampleRate, 48000);
  assert.deepEqual(take.beats, [{ sampleIndex: 1000, beatInBar: 0 }]);
  assert.deepEqual(take.onsets, [{ sampleIndex: 1200, strength: 6.5 }]);
  assert.deepEqual(take.pitchFrames, [{ sampleIndex: 1300, frequencyHz: 196, confidence: 0.9 }]);
});

test("accumulates across batches and keeps the worst dropped count", () => {
  const take = createTake();
  appendBatch(take, { sampleRate: 44100, dropped: 3, events: [[EVENT_KIND.onset, 1, 4, 1]] });
  appendBatch(take, { sampleRate: 44100, dropped: 1, events: [[EVENT_KIND.onset, 2, 4, 1]] });
  assert.equal(take.onsets.length, 2);
  assert.equal(take.droppedEvents, 3);
});

test("skips malformed tuples without losing the rest", () => {
  const take = appendBatch(createTake(), {
    sampleRate: 48000,
    events: [null, [EVENT_KIND.onset], [EVENT_KIND.onset, "x", 1], [EVENT_KIND.onset, 50, 4], [99, 60, 1, 1]],
  });
  assert.deepEqual(take.onsets, [{ sampleIndex: 50, strength: 4 }]);
});

test("ignores batches with no events and invalid sample rates", () => {
  const take = createTake();
  appendBatch(take, null);
  appendBatch(take, { sampleRate: -1, events: [] });
  assert.equal(take.sampleRate, 0);
});

test("pitch frames convert to milliseconds from the first frame", () => {
  const take = appendBatch(createTake(), {
    sampleRate: 48000,
    events: [
      [EVENT_KIND.pitch, 48000, 220, 1],
      [EVENT_KIND.pitch, 48480, 0, 0.1],
    ],
  });
  assert.deepEqual(pitchFramesInMs(take), [
    { timeMs: 0, frequencyHz: 220 },
    { timeMs: 10, frequencyHz: 0 },
  ]);
  assert.deepEqual(pitchFramesInMs(createTake()), []);
});
