// Practice log: sessions, targets and trends.
//
// The C++ side stores this document verbatim (Source/practice/PracticeLogStore), so this
// module owns the schema. Everything is pure and immutable: functions return a new log,
// which keeps the UI simple and the behaviour easy to test.
(function attach(root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) module.exports = api;
  else root.LumenPractice = Object.assign(root.LumenPractice || {}, { log: api });
})(typeof self !== "undefined" ? self : this, function createPracticeLogModule() {
  "use strict";

  const SCHEMA_VERSION = 1;
  // Roughly a year of several sessions a day; keeps the file small and saves fast.
  const MAX_SESSIONS = 1500;
  const SESSION_KINDS = Object.freeze(["timing", "vibrato", "fretboard"]);
  const MILLISECONDS_PER_DAY = 24 * 60 * 60 * 1000;
  const MAX_NOTE_LENGTH = 500;

  const DEFAULT_TARGETS = Object.freeze({
    timing: Object.freeze({ bpm: 90, subdivision: 4, toleranceMs: 15 }),
    // A starting point, not a rule: set your own from players you want to sound like.
    vibrato: Object.freeze({ rateHz: 5.5, widthCents: 50 }),
  });

  const DEFAULT_SETTINGS = Object.freeze({
    // Added on top of the device-reported latency; found by the calibration routine.
    latencyOffsetMs: 0,
  });

  function createEmptyLog() {
    return {
      version: SCHEMA_VERSION,
      sessions: [],
      targets: structuredCloneSafe(DEFAULT_TARGETS),
      settings: { ...DEFAULT_SETTINGS },
    };
  }

  // structuredClone is missing in some older WebViews; the data here is plain JSON.
  function structuredCloneSafe(value) {
    return JSON.parse(JSON.stringify(value));
  }

  function isPlainObject(value) {
    return value !== null && typeof value === "object" && !Array.isArray(value);
  }

  function finiteOr(value, fallback) {
    const number = Number(value);
    return Number.isFinite(number) ? number : fallback;
  }

  function normaliseMetrics(metrics) {
    if (!isPlainObject(metrics)) return {};
    return Object.fromEntries(
      Object.entries(metrics).filter(([, value]) => Number.isFinite(value) || typeof value === "string" || typeof value === "boolean")
    );
  }

  function normaliseSession(raw) {
    if (!isPlainObject(raw) || !SESSION_KINDS.includes(raw.kind)) return null;
    const startedAt = new Date(raw.startedAt);
    if (Number.isNaN(startedAt.getTime())) return null;
    return {
      id: typeof raw.id === "string" && raw.id ? raw.id : `${raw.kind}-${startedAt.getTime()}`,
      kind: raw.kind,
      startedAt: startedAt.toISOString(),
      durationSec: Math.max(0, finiteOr(raw.durationSec, 0)),
      metrics: normaliseMetrics(raw.metrics),
      note: typeof raw.note === "string" ? raw.note.slice(0, MAX_NOTE_LENGTH) : "",
    };
  }

  // Accepts whatever came off disk (possibly missing, old or hand-edited) and returns a
  // valid log. Bad sessions are dropped individually instead of discarding the whole file.
  function normaliseLog(raw) {
    const log = createEmptyLog();
    if (!isPlainObject(raw)) return log;

    const sessions = Array.isArray(raw.sessions) ? raw.sessions.map(normaliseSession).filter(Boolean) : [];
    sessions.sort((a, b) => a.startedAt.localeCompare(b.startedAt));
    log.sessions = sessions.slice(-MAX_SESSIONS);

    if (isPlainObject(raw.targets)) {
      const timing = isPlainObject(raw.targets.timing) ? raw.targets.timing : {};
      const vibrato = isPlainObject(raw.targets.vibrato) ? raw.targets.vibrato : {};
      log.targets.timing = {
        bpm: finiteOr(timing.bpm, DEFAULT_TARGETS.timing.bpm),
        subdivision: finiteOr(timing.subdivision, DEFAULT_TARGETS.timing.subdivision),
        toleranceMs: finiteOr(timing.toleranceMs, DEFAULT_TARGETS.timing.toleranceMs),
      };
      log.targets.vibrato = {
        rateHz: finiteOr(vibrato.rateHz, DEFAULT_TARGETS.vibrato.rateHz),
        widthCents: finiteOr(vibrato.widthCents, DEFAULT_TARGETS.vibrato.widthCents),
      };
    }

    if (isPlainObject(raw.settings)) {
      log.settings.latencyOffsetMs = finiteOr(raw.settings.latencyOffsetMs, DEFAULT_SETTINGS.latencyOffsetMs);
    }
    return log;
  }

  function addSession(log, session) {
    const normalised = normaliseSession(session);
    if (!normalised) throw new Error("Invalid practice session");
    const sessions = [...log.sessions, normalised].sort((a, b) => a.startedAt.localeCompare(b.startedAt));
    return { ...log, sessions: sessions.slice(-MAX_SESSIONS) };
  }

  function deleteSession(log, sessionId) {
    return { ...log, sessions: log.sessions.filter((session) => session.id !== sessionId) };
  }

  function setTargets(log, kind, targets) {
    return { ...log, targets: { ...log.targets, [kind]: { ...log.targets[kind], ...targets } } };
  }

  function setSettings(log, settings) {
    return { ...log, settings: { ...log.settings, ...settings } };
  }

  // Local calendar day key, so a late-night session counts for the day the player felt it was.
  function dayKey(date) {
    const local = new Date(date);
    const month = String(local.getMonth() + 1).padStart(2, "0");
    const day = String(local.getDate()).padStart(2, "0");
    return `${local.getFullYear()}-${month}-${day}`;
  }

  // One point per practice day: the day's average of a numeric metric.
  function trend(log, kind, metricKey, { days = 30, now = new Date() } = {}) {
    const since = now.getTime() - days * MILLISECONDS_PER_DAY;
    const byDay = new Map();
    for (const session of log.sessions) {
      if (session.kind !== kind || new Date(session.startedAt).getTime() < since) continue;
      const value = session.metrics[metricKey];
      if (!Number.isFinite(value)) continue;
      const key = dayKey(session.startedAt);
      const bucket = byDay.get(key) || [];
      bucket.push(value);
      byDay.set(key, bucket);
    }
    return [...byDay.entries()]
      .sort(([a], [b]) => a.localeCompare(b))
      .map(([day, values]) => ({ day, value: values.reduce((sum, value) => sum + value, 0) / values.length, count: values.length }));
  }

  // Consecutive days (ending today or yesterday) with at least one session.
  // Ending yesterday still counts so the streak does not look broken before today's practice.
  function currentStreakDays(log, now = new Date()) {
    const practisedDays = new Set(log.sessions.map((session) => dayKey(session.startedAt)));
    const cursor = new Date(now);
    if (!practisedDays.has(dayKey(cursor))) cursor.setDate(cursor.getDate() - 1);
    let streak = 0;
    while (practisedDays.has(dayKey(cursor))) {
      streak += 1;
      cursor.setDate(cursor.getDate() - 1);
    }
    return streak;
  }

  function totals(log, { days = 7, now = new Date() } = {}) {
    const since = now.getTime() - days * MILLISECONDS_PER_DAY;
    const recent = log.sessions.filter((session) => new Date(session.startedAt).getTime() >= since);
    const byKind = Object.fromEntries(SESSION_KINDS.map((kind) => [kind, 0]));
    let seconds = 0;
    for (const session of recent) {
      byKind[session.kind] += 1;
      seconds += session.durationSec;
    }
    return { sessionCount: recent.length, minutes: Math.round(seconds / 60), byKind };
  }

  return {
    SCHEMA_VERSION,
    MAX_SESSIONS,
    SESSION_KINDS,
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
    dayKey,
  };
});
