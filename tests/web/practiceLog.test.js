const test = require("node:test");
const assert = require("node:assert/strict");
const practiceLog = require("../../Resources/web/practice/practiceLog.js");

const {
  MAX_SESSIONS,
  DEFAULT_TARGETS,
  createEmptyLog,
  normaliseLog,
  addSession,
  deleteSession,
  setTargets,
  setSettings,
  trend,
  currentStreakDays,
  totals,
} = practiceLog;

// Noon local time avoids day-boundary surprises in any timezone.
const dayAt = (year, month, day, hour = 12) => new Date(year, month - 1, day, hour).toISOString();

function session(kind, startedAt, metrics = {}, extra = {}) {
  return { id: `${kind}-${startedAt}`, kind, startedAt, durationSec: 300, metrics, ...extra };
}

test("missing or garbage input normalises to an empty log", () => {
  for (const raw of [undefined, null, 42, "x", [], { sessions: "nope" }]) {
    const log = normaliseLog(raw);
    assert.deepEqual(log.sessions, []);
    assert.deepEqual(log.targets, createEmptyLog().targets);
  }
});

test("drops invalid sessions individually and sorts the rest", () => {
  const log = normaliseLog({
    sessions: [
      session("vibrato", dayAt(2026, 10, 3)),
      { kind: "unknown", startedAt: dayAt(2026, 10, 1) },
      { kind: "timing", startedAt: "not a date" },
      session("timing", dayAt(2026, 10, 1), { score: 80, bad: { nested: true } }),
    ],
  });
  assert.deepEqual(log.sessions.map((entry) => entry.kind), ["timing", "vibrato"]);
  assert.deepEqual(log.sessions[0].metrics, { score: 80 });
});

test("caps stored sessions, keeping the newest", () => {
  const sessions = Array.from({ length: MAX_SESSIONS + 10 }, (_, index) =>
    session("timing", new Date(Date.UTC(2025, 0, 1) + index * 60000).toISOString())
  );
  const log = normaliseLog({ sessions });
  assert.equal(log.sessions.length, MAX_SESSIONS);
  assert.equal(log.sessions[log.sessions.length - 1].startedAt, sessions[sessions.length - 1].startedAt);
});

test("bad targets and settings fall back to defaults", () => {
  const log = normaliseLog({ targets: { timing: { bpm: "fast" }, vibrato: { rateHz: 6 } }, settings: { latencyOffsetMs: "?" } });
  assert.equal(log.targets.timing.bpm, DEFAULT_TARGETS.timing.bpm);
  assert.equal(log.targets.vibrato.rateHz, 6);
  assert.equal(log.targets.vibrato.widthCents, DEFAULT_TARGETS.vibrato.widthCents);
  assert.equal(log.settings.latencyOffsetMs, 0);
});

test("long notes are truncated", () => {
  const log = addSession(createEmptyLog(), session("fretboard", dayAt(2026, 10, 1), {}, { note: "x".repeat(2000) }));
  assert.equal(log.sessions[0].note.length, 500);
});

test("adding and deleting sessions never mutates the original", () => {
  const empty = createEmptyLog();
  const withOne = addSession(empty, session("timing", dayAt(2026, 10, 1), { score: 70 }));
  assert.equal(empty.sessions.length, 0);
  assert.equal(withOne.sessions.length, 1);
  const withNone = deleteSession(withOne, withOne.sessions[0].id);
  assert.equal(withOne.sessions.length, 1);
  assert.equal(withNone.sessions.length, 0);
  assert.throws(() => addSession(empty, { kind: "nope" }), /Invalid practice session/);
});

test("targets and settings update immutably", () => {
  const log = createEmptyLog();
  const updated = setSettings(setTargets(log, "vibrato", { rateHz: 6 }), { latencyOffsetMs: 14 });
  assert.equal(updated.targets.vibrato.rateHz, 6);
  assert.equal(updated.targets.vibrato.widthCents, DEFAULT_TARGETS.vibrato.widthCents);
  assert.equal(updated.settings.latencyOffsetMs, 14);
  assert.equal(log.targets.vibrato.rateHz, DEFAULT_TARGETS.vibrato.rateHz);
});

test("trend averages each practice day and respects the window", () => {
  let log = createEmptyLog();
  log = addSession(log, session("timing", dayAt(2026, 10, 1, 9), { score: 60 }));
  log = addSession(log, session("timing", dayAt(2026, 10, 1, 20), { score: 80 }));
  log = addSession(log, session("timing", dayAt(2026, 10, 3), { score: 90 }));
  log = addSession(log, session("vibrato", dayAt(2026, 10, 3), { score: 10 }));
  log = addSession(log, session("timing", dayAt(2026, 8, 1), { score: 5 }));

  const points = trend(log, "timing", "score", { days: 30, now: new Date(2026, 9, 4) });
  assert.deepEqual(points, [
    { day: "2026-10-01", value: 70, count: 2 },
    { day: "2026-10-03", value: 90, count: 1 },
  ]);
});

test("streak counts consecutive days, allowing today to be pending", () => {
  let log = createEmptyLog();
  for (const day of [1, 2, 3, 5, 6]) log = addSession(log, session("fretboard", dayAt(2026, 10, day)));

  assert.equal(currentStreakDays(log, new Date(2026, 9, 6, 18)), 2);
  assert.equal(currentStreakDays(log, new Date(2026, 9, 7, 9)), 2, "yesterday still keeps the streak alive");
  assert.equal(currentStreakDays(log, new Date(2026, 9, 8, 9)), 0);
  assert.equal(currentStreakDays(log, new Date(2026, 9, 3, 9)), 3);
});

test("totals sum recent minutes and sessions by kind", () => {
  let log = createEmptyLog();
  log = addSession(log, session("timing", dayAt(2026, 10, 5)));
  log = addSession(log, session("vibrato", dayAt(2026, 10, 6)));
  log = addSession(log, session("vibrato", dayAt(2026, 9, 1)));
  const result = totals(log, { days: 7, now: new Date(2026, 9, 7) });
  assert.equal(result.sessionCount, 2);
  assert.equal(result.minutes, 10);
  assert.deepEqual(result.byKind, { timing: 1, vibrato: 1, fretboard: 0 });
});

test("a saved log survives a JSON round trip", () => {
  let log = createEmptyLog();
  log = addSession(log, session("timing", dayAt(2026, 10, 5), { score: 88, tendency: "rushing" }));
  log = setSettings(log, { latencyOffsetMs: 9.5 });
  assert.deepEqual(normaliseLog(JSON.parse(JSON.stringify(log))), log);
});
