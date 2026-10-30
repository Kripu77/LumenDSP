// Fretboard theory: scales and modes mapped onto the neck, spelled correctly, plus a quiz.
//
// Spelling matters for learning: in F Lydian the #4 is B (not Cb), in Bb Dorian the b3 is
// Db (not C#). Each scale degree carries its letter step, so names come out the way a
// chart would print them.
(function attach(root, factory) {
  const api = factory();
  if (typeof module === "object" && module.exports) module.exports = api;
  else root.LumenPractice = Object.assign(root.LumenPractice || {}, { fretboard: api });
})(typeof self !== "undefined" ? self : this, function createFretboardModule() {
  "use strict";

  const SEMITONES_PER_OCTAVE = 12;
  const LETTERS = ["C", "D", "E", "F", "G", "A", "B"];
  const LETTER_PITCH_CLASSES = [0, 2, 4, 5, 7, 9, 11];
  const DEFAULT_FRET_COUNT = 22;

  // Roots offered in the UI, spelled the way they are normally read.
  const ROOTS = Object.freeze(["C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"]);

  // Low string first, as MIDI note numbers.
  const TUNINGS = Object.freeze({
    standard: { label: "Standard (EADGBE)", midi: [40, 45, 50, 55, 59, 64] },
    dropD: { label: "Drop D (DADGBE)", midi: [38, 45, 50, 55, 59, 64] },
    halfStepDown: { label: "Eb standard", midi: [39, 44, 49, 54, 58, 63] },
    dStandard: { label: "D standard", midi: [38, 43, 48, 53, 57, 62] },
  });

  // Degree labels are the source of truth: semitones and spelling are derived from them.
  // `colour` names the degrees that give each mode its sound, used in quiz questions.
  const SCALES = Object.freeze({
    ionian: { label: "Ionian (major)", family: "Major modes", degrees: ["1", "2", "3", "4", "5", "6", "7"], colour: ["3", "7"] },
    dorian: { label: "Dorian", family: "Major modes", degrees: ["1", "2", "b3", "4", "5", "6", "b7"], colour: ["6"] },
    phrygian: { label: "Phrygian", family: "Major modes", degrees: ["1", "b2", "b3", "4", "5", "b6", "b7"], colour: ["b2"] },
    lydian: { label: "Lydian", family: "Major modes", degrees: ["1", "2", "3", "#4", "5", "6", "7"], colour: ["#4"] },
    mixolydian: { label: "Mixolydian", family: "Major modes", degrees: ["1", "2", "3", "4", "5", "6", "b7"], colour: ["b7"] },
    aeolian: { label: "Aeolian (natural minor)", family: "Major modes", degrees: ["1", "2", "b3", "4", "5", "b6", "b7"], colour: ["b6"] },
    locrian: { label: "Locrian", family: "Major modes", degrees: ["1", "b2", "b3", "4", "b5", "b6", "b7"], colour: ["b5"] },

    melodicMinor: { label: "Melodic minor", family: "Melodic minor modes", degrees: ["1", "2", "b3", "4", "5", "6", "7"], colour: ["b3", "7"] },
    dorianFlat2: { label: "Dorian b2", family: "Melodic minor modes", degrees: ["1", "b2", "b3", "4", "5", "6", "b7"], colour: ["b2", "6"] },
    lydianAugmented: { label: "Lydian augmented", family: "Melodic minor modes", degrees: ["1", "2", "3", "#4", "#5", "6", "7"], colour: ["#4", "#5"] },
    lydianDominant: { label: "Lydian dominant", family: "Melodic minor modes", degrees: ["1", "2", "3", "#4", "5", "6", "b7"], colour: ["#4", "b7"] },
    mixolydianFlat6: { label: "Mixolydian b6", family: "Melodic minor modes", degrees: ["1", "2", "3", "4", "5", "b6", "b7"], colour: ["3", "b6"] },
    locrianNatural2: { label: "Locrian #2", family: "Melodic minor modes", degrees: ["1", "2", "b3", "4", "b5", "b6", "b7"], colour: ["2", "b5"] },
    altered: { label: "Altered (super Locrian)", family: "Melodic minor modes", degrees: ["1", "b2", "#2", "3", "b5", "#5", "b7"], colour: ["b2", "#2", "b5", "#5"] },

    harmonicMinor: { label: "Harmonic minor", family: "Other", degrees: ["1", "2", "b3", "4", "5", "b6", "7"], colour: ["b6", "7"] },
    phrygianDominant: { label: "Phrygian dominant", family: "Other", degrees: ["1", "b2", "3", "4", "5", "b6", "b7"], colour: ["b2", "3"] },
    majorPentatonic: { label: "Major pentatonic", family: "Pentatonic & hexatonic", degrees: ["1", "2", "3", "5", "6"], colour: [] },
    minorPentatonic: { label: "Minor pentatonic", family: "Pentatonic & hexatonic", degrees: ["1", "b3", "4", "5", "b7"], colour: [] },
    blues: { label: "Blues", family: "Pentatonic & hexatonic", degrees: ["1", "b3", "4", "b5", "5", "b7"], colour: ["b5"] },
    minorHexatonic: { label: "Minor hexatonic (Dorian/Aeolian, no 6)", family: "Pentatonic & hexatonic", degrees: ["1", "2", "b3", "4", "5", "b7"], colour: ["2"] },
    majorHexatonic: { label: "Major hexatonic (no 4)", family: "Pentatonic & hexatonic", degrees: ["1", "2", "3", "5", "6", "7"], colour: ["7"] },
    wholeTone: { label: "Whole tone", family: "Symmetric", degrees: ["1", "2", "3", "#4", "#5", "b7"], colour: ["#4", "#5"] },
    diminishedHalfWhole: { label: "Diminished (half-whole)", family: "Symmetric", degrees: ["1", "b2", "#2", "3", "#4", "5", "6", "b7"], colour: ["b2", "#2", "#4"] },
  });

  // Semitones above the root for each natural degree number.
  const MAJOR_SCALE_SEMITONES = { 1: 0, 2: 2, 3: 4, 4: 5, 5: 7, 6: 9, 7: 11 };

  function modulo(value, divisor) {
    return ((value % divisor) + divisor) % divisor;
  }

  function parseDegree(label) {
    const match = /^(b{0,2}|#{0,2})([1-7])$/.exec(label);
    if (!match) throw new Error(`Bad degree label: ${label}`);
    const accidental = match[1];
    const number = Number(match[2]);
    const alteration = accidental.startsWith("b") ? -accidental.length : accidental.length;
    return { number, semitones: modulo(MAJOR_SCALE_SEMITONES[number] + alteration, SEMITONES_PER_OCTAVE) };
  }

  function parseNoteName(name) {
    const match = /^([A-G])(b{0,2}|#{0,2})$/.exec(name);
    if (!match) throw new Error(`Bad note name: ${name}`);
    const letterIndex = LETTERS.indexOf(match[1]);
    const alteration = match[2].startsWith("b") ? -match[2].length : match[2].length;
    return { letterIndex, pitchClass: modulo(LETTER_PITCH_CLASSES[letterIndex] + alteration, SEMITONES_PER_OCTAVE) };
  }

  function spell(letterIndex, pitchClass) {
    const natural = LETTER_PITCH_CLASSES[letterIndex];
    // Smallest signed distance from the natural letter to the target pitch class.
    const offset = modulo(pitchClass - natural + 6, SEMITONES_PER_OCTAVE) - 6;
    const accidental = offset < 0 ? "b".repeat(-offset) : "#".repeat(offset);
    return LETTERS[letterIndex] + accidental;
  }

  // The scale as spelled notes: [{ degree, pitchClass, name }].
  function scaleNotes(rootName, scaleId) {
    const scale = SCALES[scaleId];
    if (!scale) throw new Error(`Unknown scale: ${scaleId}`);
    const root = parseNoteName(rootName);
    return scale.degrees.map((degree) => {
      const { number, semitones } = parseDegree(degree);
      const pitchClass = modulo(root.pitchClass + semitones, SEMITONES_PER_OCTAVE);
      const letterIndex = modulo(root.letterIndex + number - 1, LETTERS.length);
      return { degree, pitchClass, name: spell(letterIndex, pitchClass) };
    });
  }

  // Name for a pitch class outside the scale: follow the root's flat/sharp leaning.
  function chromaticName(pitchClass, rootName) {
    const prefersFlats = rootName.includes("b") || rootName === "F";
    const sharps = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
    const flats = ["C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"];
    return (prefersFlats ? flats : sharps)[pitchClass];
  }

  // strings[stringIndex][fret] = { midi, pitchClass, name, inScale, degree, isRoot }.
  // stringIndex 0 is the LOW string.
  function buildFretboard({ rootName, scaleId, tuningId = "standard", fretCount = DEFAULT_FRET_COUNT }) {
    const tuning = TUNINGS[tuningId];
    if (!tuning) throw new Error(`Unknown tuning: ${tuningId}`);
    const notes = scaleNotes(rootName, scaleId);
    const byPitchClass = new Map(notes.map((note) => [note.pitchClass, note]));
    const rootPitchClass = parseNoteName(rootName).pitchClass;

    return tuning.midi.map((openMidi) =>
      Array.from({ length: fretCount + 1 }, (_, fret) => {
        const midi = openMidi + fret;
        const pitchClass = modulo(midi, SEMITONES_PER_OCTAVE);
        const scaleNote = byPitchClass.get(pitchClass);
        return {
          midi,
          pitchClass,
          name: scaleNote ? scaleNote.name : chromaticName(pitchClass, rootName),
          inScale: Boolean(scaleNote),
          degree: scaleNote ? scaleNote.degree : null,
          isRoot: pitchClass === rootPitchClass,
        };
      })
    );
  }

  // ---- Quiz -------------------------------------------------------------------------

  const QUESTION_TYPES = Object.freeze(["degreeAtFret", "noteAtFret", "modeFromColour", "noteForDegree"]);
  const CHOICE_COUNT = 4;

  function pick(items, random) {
    return items[Math.floor(random() * items.length)];
  }

  function shuffle(items, random) {
    const copy = [...items];
    for (let index = copy.length - 1; index > 0; index -= 1) {
      const swap = Math.floor(random() * (index + 1));
      [copy[index], copy[swap]] = [copy[swap], copy[index]];
    }
    return copy;
  }

  function choicesWith(answer, pool, random) {
    const distractors = shuffle([...new Set(pool)].filter((item) => item !== answer), random).slice(0, CHOICE_COUNT - 1);
    return shuffle([answer, ...distractors], random);
  }

  function inScalePositions(board, minFret, maxFret) {
    const positions = [];
    board.forEach((string, stringIndex) => {
      for (let fret = minFret; fret <= Math.min(maxFret, string.length - 1); fret += 1) {
        if (string[fret].inScale) positions.push({ stringIndex, fret, cell: string[fret] });
      }
    });
    return positions;
  }

  // Generates one multiple-choice question. `random` is injectable so tests are deterministic.
  // context: { rootName, scaleId, tuningId, minFret, maxFret, types }
  function createQuestion(context, random = Math.random) {
    const { rootName, scaleId, tuningId = "standard", minFret = 0, maxFret = 12, types = QUESTION_TYPES } = context;
    const type = pick(types, random);
    const scale = SCALES[scaleId];

    if (type === "modeFromColour") {
      const candidates = Object.entries(SCALES).filter(([, entry]) => entry.colour.length && entry.degrees.length === 7);
      const [answerId, answerScale] = pick(candidates, random);
      const sameFamily = candidates.filter(([, entry]) => entry.family === answerScale.family).map(([id]) => id);
      return {
        type,
        prompt: `Which ${answerScale.family.replace(/ modes$/, "").toLowerCase()} mode is defined by ${answerScale.colour.join(" and ")}?`,
        choices: choicesWith(answerId, sameFamily, random).map((id) => ({ id, label: SCALES[id].label })),
        answer: answerId,
        highlight: null,
      };
    }

    if (type === "noteForDegree") {
      const notes = scaleNotes(rootName, scaleId);
      const target = pick(notes.filter((note) => note.degree !== "1"), random);
      return {
        type,
        prompt: `In ${rootName} ${scale.label}, what is the ${target.degree}?`,
        choices: choicesWith(target.name, notes.map((note) => note.name), random).map((name) => ({ id: name, label: name })),
        answer: target.name,
        highlight: null,
      };
    }

    const board = buildFretboard({ rootName, scaleId, tuningId, fretCount: Math.max(maxFret, 1) });
    const positions = inScalePositions(board, minFret, maxFret);
    if (positions.length === 0) return createQuestion({ ...context, types: ["noteForDegree"] }, random);
    const position = pick(positions, random);

    if (type === "noteAtFret") {
      return {
        type,
        prompt: `Name the highlighted note (${rootName} ${scale.label}).`,
        choices: choicesWith(position.cell.name, scaleNotes(rootName, scaleId).map((note) => note.name), random).map((name) => ({ id: name, label: name })),
        answer: position.cell.name,
        highlight: { stringIndex: position.stringIndex, fret: position.fret },
      };
    }

    return {
      type: "degreeAtFret",
      prompt: `Which degree of ${rootName} ${scale.label} is highlighted?`,
      choices: choicesWith(position.cell.degree, scale.degrees, random).map((degree) => ({ id: degree, label: degree })),
      answer: position.cell.degree,
      highlight: { stringIndex: position.stringIndex, fret: position.fret },
    };
  }

  function gradeAnswer(question, choiceId) {
    return { isCorrect: question.answer === choiceId, answer: question.answer };
  }

  // Small seeded generator (mulberry32) for reproducible quizzes and tests.
  function seededRandom(seed) {
    let state = seed >>> 0;
    return function next() {
      state = (state + 0x6d2b79f5) >>> 0;
      let mixed = Math.imul(state ^ (state >>> 15), 1 | state);
      mixed = (mixed + Math.imul(mixed ^ (mixed >>> 7), 61 | mixed)) ^ mixed;
      return ((mixed ^ (mixed >>> 14)) >>> 0) / 4294967296;
    };
  }

  return {
    ROOTS,
    TUNINGS,
    SCALES,
    QUESTION_TYPES,
    scaleNotes,
    buildFretboard,
    createQuestion,
    gradeAnswer,
    seededRandom,
    parseNoteName,
  };
});
