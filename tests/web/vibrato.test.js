const test = require("node:test");
const assert = require("node:assert/strict");
const { analyseVibrato, scoreAgainstTarget, hzToCents, noteNameForCents } = require("../../Resources/web/practice/vibrato.js");

// The tuner hops 512 samples at 48 kHz: ~93.75 frames per second.
const FRAME_MS = (512 / 48000) * 1000;

// Builds a frame stream for one held note. Pitch functions return cents relative to baseHz.
function heldNote({ baseHz = 196, durationMs = 1500, startMs = 0, centsAt = () => 0 }) {
  const frames = [];
  for (let time = 0; time <= durationMs; time += FRAME_MS) {
    frames.push({ timeMs: startMs + time, frequencyHz: baseHz * 2 ** (centsAt(time) / 1200) });
  }
  return frames;
}

const sine = (rateHz, peakToPeakCents) => (timeMs) => (peakToPeakCents / 2) * Math.sin((2 * Math.PI * rateHz * timeMs) / 1000);

test("a straight held note has no vibrato", () => {
  const { notes, summary } = analyseVibrato(heldNote({}));
  assert.equal(notes.length, 1);
  assert.equal(notes[0].hasVibrato, false);
  assert.equal(summary, null);
});

test("measures rate and width of an even vibrato", () => {
  const { vibratoNotes, summary } = analyseVibrato(heldNote({ centsAt: sine(5.5, 50) }));
  assert.equal(vibratoNotes.length, 1);
  assert.ok(Math.abs(summary.rateHz - 5.5) < 0.25, `rate ${summary.rateHz}`);
  assert.ok(Math.abs(summary.widthCents - 50) < 3, `width ${summary.widthCents}`);
  assert.ok(summary.consistency >= 80, `consistency ${summary.consistency}`);
});

test("measures a wide slow vibrato as well as a narrow fast one", () => {
  const slowWide = analyseVibrato(heldNote({ centsAt: sine(4, 90), durationMs: 2000 })).summary;
  const fastNarrow = analyseVibrato(heldNote({ centsAt: sine(7, 25) })).summary;
  assert.ok(Math.abs(slowWide.rateHz - 4) < 0.3, `slow rate ${slowWide.rateHz}`);
  assert.ok(Math.abs(slowWide.widthCents - 90) < 4, `slow width ${slowWide.widthCents}`);
  assert.ok(Math.abs(fastNarrow.rateHz - 7) < 0.3, `fast rate ${fastNarrow.rateHz}`);
  assert.ok(Math.abs(fastNarrow.widthCents - 25) < 2, `fast width ${fastNarrow.widthCents}`);
});

test("vibrato on top of a bend is measured without the bend", () => {
  // Bend up a whole step over the first 400 ms, then hold it with vibrato.
  const bend = (timeMs) => Math.min(200, (timeMs / 400) * 200);
  const centsAt = (timeMs) => bend(timeMs) + (timeMs > 400 ? sine(5, 60)(timeMs) : 0);
  // The bend itself rises ~20 cents per frame at most, so it stays one note.
  const { vibratoNotes } = analyseVibrato(heldNote({ centsAt, durationMs: 1800 }));
  assert.equal(vibratoNotes.length, 1);
  assert.ok(Math.abs(vibratoNotes[0].rateHz - 5) < 0.4, `rate ${vibratoNotes[0].rateHz}`);
  assert.ok(Math.abs(vibratoNotes[0].widthCents - 60) < 5, `width ${vibratoNotes[0].widthCents}`);
});

test("an uneven vibrato scores lower consistency than an even one", () => {
  // Alternate between 4 Hz and 7 Hz cycles: the wobble of an uncontrolled wrist.
  const uneven = (timeMs) => {
    const cycleMs = [250, 143];
    let elapsed = timeMs;
    let index = 0;
    while (elapsed > cycleMs[index % 2]) elapsed -= cycleMs[index++ % 2];
    return 25 * Math.sin((2 * Math.PI * elapsed) / cycleMs[index % 2]);
  };
  const even = analyseVibrato(heldNote({ centsAt: sine(5.5, 50), durationMs: 2000 })).summary;
  const wobbly = analyseVibrato(heldNote({ centsAt: uneven, durationMs: 2000 })).summary;
  assert.ok(wobbly.consistency < even.consistency - 20, `even ${even.consistency} vs wobbly ${wobbly.consistency}`);
});

test("splits separate notes at silences and pitch jumps", () => {
  const first = heldNote({ baseHz: 196, durationMs: 800, centsAt: sine(5, 40) });
  const silence = [{ timeMs: 820, frequencyHz: 0 }, { timeMs: 900, frequencyHz: 0 }];
  const second = heldNote({ baseHz: 220, durationMs: 800, startMs: 950, centsAt: sine(6, 40) });
  const third = heldNote({ baseHz: 262, durationMs: 800, startMs: 1760, centsAt: sine(6, 40) });
  const { notes } = analyseVibrato([...first, ...silence, ...second, ...third]);
  assert.deepEqual(notes.map((note) => note.noteName), ["G3", "A3", "C4"]);
});

test("a one-frame tuner dropout does not split a note", () => {
  const frames = heldNote({ centsAt: sine(5, 40) });
  frames[40] = { ...frames[40], frequencyHz: 0 };
  assert.equal(analyseVibrato(frames).notes.length, 1);
});

test("notes too short to judge are left out", () => {
  const { notes } = analyseVibrato(heldNote({ durationMs: 300, centsAt: sine(6, 50) }));
  assert.equal(notes.length, 0);
});

test("reports intonation separately from vibrato", () => {
  const { notes } = analyseVibrato(heldNote({ baseHz: 196 * 2 ** (20 / 1200) }));
  assert.ok(Math.abs(notes[0].intonationCents - 20) < 1);
  assert.equal(notes[0].noteName, "G3");
});

test("empty and unvoiced input are handled", () => {
  assert.deepEqual(analyseVibrato([]), { notes: [], vibratoNotes: [], summary: null });
  assert.deepEqual(analyseVibrato(null).notes, []);
  assert.equal(analyseVibrato([{ timeMs: 0, frequencyHz: 0 }, { timeMs: 10, frequencyHz: 0 }]).notes.length, 0);
});

test("target score rewards matching rate, width and consistency", () => {
  const target = { rateHz: 5.5, widthCents: 50 };
  const onTarget = scoreAgainstTarget({ rateHz: 5.5, widthCents: 50, consistency: 100 }, target);
  const tooFast = scoreAgainstTarget({ rateHz: 8, widthCents: 50, consistency: 100 }, target);
  const tooNarrow = scoreAgainstTarget({ rateHz: 5.5, widthCents: 20, consistency: 100 }, target);
  assert.equal(onTarget, 100);
  assert.ok(tooFast < onTarget && tooNarrow < onTarget);
  assert.equal(scoreAgainstTarget(null, target), 0);
  assert.equal(scoreAgainstTarget({ rateHz: 5, widthCents: 40, consistency: 90 }, { rateHz: 0, widthCents: 40 }), 0);
});

test("cent and note helpers agree with concert pitch", () => {
  assert.equal(hzToCents(440), 0);
  assert.equal(Math.round(hzToCents(880)), 1200);
  assert.equal(noteNameForCents(0), "A4");
  assert.equal(noteNameForCents(hzToCents(82.41)), "E2");
});

// Real tuner frames each average pitch over a 2048-sample window (~42.7 ms at 48 kHz),
// which shrinks the measured swing. Simulate that averaging and check the correction.
function windowedNote({ rateHz, peakToPeakCents, windowMs, durationMs = 1600, baseHz = 196 }) {
  const frames = [];
  const sub = 32;
  for (let time = windowMs / 2; time <= durationMs; time += FRAME_MS) {
    let sum = 0;
    for (let step = 0; step < sub; step += 1) {
      const t = time - windowMs / 2 + (step + 0.5) * (windowMs / sub);
      sum += (peakToPeakCents / 2) * Math.sin((2 * Math.PI * rateHz * t) / 1000);
    }
    frames.push({ timeMs: time, frequencyHz: baseHz * 2 ** (sum / sub / 1200) });
  }
  return frames;
}

test("corrects the width lost to the tuner's analysis window", () => {
  const windowMs = (2048 / 48000) * 1000;
  for (const rateHz of [5, 7]) {
    const frames = windowedNote({ rateHz, peakToPeakCents: 50, windowMs });
    const raw = analyseVibrato(frames).summary.widthCents;
    const corrected = analyseVibrato(frames, { analysisWindowMs: windowMs }).summary.widthCents;
    assert.ok(raw < 48, `uncorrected should read low at ${rateHz} Hz (${raw})`);
    assert.ok(Math.abs(corrected - 50) < 3, `corrected width at ${rateHz} Hz was ${corrected}`);
  }
});

test("window correction is a no-op without a window length", () => {
  const frames = heldNote({ centsAt: sine(5.5, 50) });
  assert.deepEqual(analyseVibrato(frames, { analysisWindowMs: 0 }).summary, analyseVibrato(frames).summary);
});
