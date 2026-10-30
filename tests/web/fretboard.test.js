const test = require("node:test");
const assert = require("node:assert/strict");
const {
  ROOTS,
  SCALES,
  TUNINGS,
  QUESTION_TYPES,
  scaleNotes,
  buildFretboard,
  createQuestion,
  gradeAnswer,
  seededRandom,
  parseNoteName,
} = require("../../Resources/web/practice/fretboard.js");

const names = (rootName, scaleId) => scaleNotes(rootName, scaleId).map((note) => note.name);

test("spells the major modes correctly", () => {
  assert.deepEqual(names("C", "ionian"), ["C", "D", "E", "F", "G", "A", "B"]);
  assert.deepEqual(names("F", "lydian"), ["F", "G", "A", "B", "C", "D", "E"]);
  assert.deepEqual(names("Bb", "dorian"), ["Bb", "C", "Db", "Eb", "F", "G", "Ab"]);
  assert.deepEqual(names("E", "phrygian"), ["E", "F", "G", "A", "B", "C", "D"]);
});

test("uses theoretically correct accidentals in sharp keys", () => {
  assert.deepEqual(names("F#", "lydian"), ["F#", "G#", "A#", "B#", "C#", "D#", "E#"]);
});

test("spells melodic minor modes the way charts print them", () => {
  assert.deepEqual(names("C", "altered"), ["C", "Db", "D#", "E", "Gb", "G#", "Bb"]);
  assert.deepEqual(names("G", "lydianDominant"), ["G", "A", "B", "C#", "D", "E", "F"]);
  assert.deepEqual(names("A", "melodicMinor"), ["A", "B", "C", "D", "E", "F#", "G#"]);
});

test("pentatonic and hexatonic scales keep their degree spelling", () => {
  assert.deepEqual(names("A", "minorPentatonic"), ["A", "C", "D", "E", "G"]);
  assert.deepEqual(names("E", "minorHexatonic"), ["E", "F#", "G", "A", "B", "D"]);
});

test("every spelled note matches its pitch class, for every root and scale", () => {
  for (const root of ROOTS) {
    for (const scaleId of Object.keys(SCALES)) {
      for (const note of scaleNotes(root, scaleId)) {
        assert.equal(parseNoteName(note.name).pitchClass, note.pitchClass, `${root} ${scaleId} ${note.degree}`);
      }
    }
  }
});

test("standard tuning puts the right notes on the neck", () => {
  const board = buildFretboard({ rootName: "A", scaleId: "minorPentatonic", fretCount: 12 });
  assert.equal(board.length, 6);
  assert.equal(board[0][0].name, "E"); // low E open
  assert.equal(board[0][5].name, "A");
  assert.equal(board[0][5].isRoot, true);
  assert.equal(board[0][5].degree, "1");
  assert.equal(board[5][12].midi, 76); // high E, 12th fret
  assert.equal(board[0][6].inScale, false); // A# is not in A minor pentatonic
});

test("alternate tunings shift the open strings", () => {
  const board = buildFretboard({ rootName: "D", scaleId: "dorian", tuningId: "dropD" });
  assert.equal(board[0][0].name, "D");
  assert.equal(board[0][0].isRoot, true);
  assert.equal(board[0].length, 23);
});

test("unknown scales and tunings throw", () => {
  assert.throws(() => scaleNotes("C", "nope"), /Unknown scale/);
  assert.throws(() => buildFretboard({ rootName: "C", scaleId: "ionian", tuningId: "nope" }), /Unknown tuning/);
  assert.throws(() => scaleNotes("H", "ionian"), /Bad note name/);
});

test("all scales define exactly one root and unique degrees", () => {
  for (const [id, scale] of Object.entries(SCALES)) {
    assert.equal(scale.degrees[0], "1", id);
    assert.equal(new Set(scale.degrees).size, scale.degrees.length, id);
    const pitchClasses = scaleNotes("C", id).map((note) => note.pitchClass);
    assert.equal(new Set(pitchClasses).size, pitchClasses.length, `${id} has duplicate pitches`);
  }
  assert.ok(Object.keys(TUNINGS).includes("standard"));
});

test("quiz questions are deterministic for a seed", () => {
  const context = { rootName: "E", scaleId: "lydian", minFret: 0, maxFret: 12 };
  assert.deepEqual(createQuestion(context, seededRandom(42)), createQuestion(context, seededRandom(42)));
});

test("every question type yields a valid, answerable question", () => {
  for (const type of QUESTION_TYPES) {
    for (let seed = 1; seed <= 50; seed += 1) {
      const question = createQuestion({ rootName: "G", scaleId: "dorian", minFret: 3, maxFret: 8, types: [type] }, seededRandom(seed));
      const ids = question.choices.map((choice) => choice.id);
      assert.equal(question.type, type);
      assert.ok(ids.includes(question.answer), `${type} answer missing from choices`);
      assert.equal(new Set(ids).size, ids.length, `${type} duplicate choices`);
      assert.ok(ids.length >= 2 && ids.length <= 4);
      if (question.highlight) {
        assert.ok(question.highlight.fret >= 3 && question.highlight.fret <= 8, "highlight outside the chosen frets");
      }
    }
  }
});

test("fret questions highlight a note that really has that answer", () => {
  for (let seed = 1; seed <= 30; seed += 1) {
    const context = { rootName: "C", scaleId: "melodicMinor", maxFret: 15, types: ["degreeAtFret"] };
    const question = createQuestion(context, seededRandom(seed));
    const board = buildFretboard({ rootName: "C", scaleId: "melodicMinor", fretCount: 15 });
    assert.equal(board[question.highlight.stringIndex][question.highlight.fret].degree, question.answer);
  }
});

test("grading compares against the stored answer", () => {
  const question = createQuestion({ rootName: "A", scaleId: "aeolian", types: ["noteForDegree"] }, seededRandom(7));
  assert.equal(gradeAnswer(question, question.answer).isCorrect, true);
  const wrong = question.choices.find((choice) => choice.id !== question.answer);
  assert.equal(gradeAnswer(question, wrong.id).isCorrect, false);
});
