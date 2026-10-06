import { bigCrunchReset, buyOneDimension, buyTickSpeed, GameIntervals, GameSaveSerializer, GameStorage, requestDimensionBoost, requestGalaxyReset, sacrificeReset } from "./core/globals"
import { gainedInfinityPoints } from "./game";

export const TAS = {
    isRunning: false,
    tickSwitch: true,
    startTime: null,

    instructions: [],
    currentInstruction: 0,

    lastCycleInstruction: 0,
    cycleCounter: 0,

    queue: [],

    exportedSave: null,
    segments: [],
    /*
     --- debug binary bitmask ---
     debug & 1 === logging
     
    */
    variables: {
        debug: 0,
        tick: 0
    },
    intervals: {},

    runNextPendingInstruction() {
        if (!this.isRunning) return;
        let isSuccessful = true;
        while (isSuccessful) {
            isSuccessful = this.runOneInstruction(this.currentInstruction);
            if (isSuccessful) {
                // can remove '=== 1' as it's truthy/falsey either way.
                if (TAS.variables.debug & 1 === 1) {
                    console.log(`
                        Instruction  : ${TAS.currentInstruction}, 
                        cycleCounter : ${TAS.cycleCounter},
                        tick         : ${TAS.variables.tick},
                        realtime     : ${(performance.now() - this.startTime).toFixed(0)},
                        gametime     : ${player.records.realTimePlayed}
                        `);
                };
	            this.currentInstruction += 1;
            };
        };
    },

    runOneInstruction(index) {

        if (index >= this.instructions.length) {
            this.pause();
            this.export();
            return;
        };

        let instruction = this.instructions[index];
        return instruction.run();
    },

    getSegments(pathsToInstructions, startSave) {
        TAS.segments = [];
        TAS.segments.push(startSave);
        let i = 0;
        TAS.intervals["segmentInterval"] = setInterval(async () => {
            if (!TAS.isRunning) {
                console.log(i);
                if (i >= pathsToInstructions.length) {
                    TAS.segments.push(GameStorage.exportModifiedSave());
                    console.log('TAS ended');
                    this.removeInterval("segmentInterval");
                    return;
                };
                if (TAS.exportedSave !== null) {
                    TAS.segments.push(TAS.exportedSave);
                }
                await TAS.prepareAndStart([pathsToInstructions[i]], TAS.segments[i]);
                i++;
            }
        }, 2000);
    },

    removeInterval(id) {
        clearInterval(TAS.intervals[id])
        delete TAS.intervals[id];
    },

    async prepareAndStart(pathsToInstructions, pathToSave=null) {
        await TAS.prepare(pathsToInstructions, pathToSave);
        TAS.start();
    },

    async prepare(pathsToInstructions, save=null) {
        // if it includes /input, interpret it as a fetch request
        if (typeof save === 'string' && save.includes('/input')) {
            save = await getSave(save);
        };
        TAS.reset(save);
        let i = 1;
        let max = pathsToInstructions.length;
        for (const path of pathsToInstructions) {
            console.log("Loading instructions (" + i + "/" + max + ")");
            await this.getInstructions(path);
            this.loadInstructions();
            i++;
        }
        console.log("Finished loading instructions");
    },

    importSave(save) {
        if (GameStorage.checkPlayerObject(GameSaveSerializer.deserialize(save)) === "") {
            console.log("Save found, importing");
            GameStorage.import(save);
            return true;
        } else {
            console.log("Save String is invalid, hard resetting");
            dev.hardReset();
            Speedrun.prepareSave();
        }
    },

    async getInstructions(path) {
        const response = await fetch(path);
        const text = await response.text();
        const commands = JSON.parse(text);
        TAS.queue = commands.map(([fn, args]) => {
            return {
                fn,
                args,
                action: actions[fn],
                run() {
                    return this.action(...this.args);
                }
            };
        });
        return true;
    },

    loadInstructions() {
        // push the instruction set, then flush
        TAS.instructions.push(...TAS.queue);
        TAS.queue = [];
        return true;
    },

    start() {
        console.log("TAS started running");
        this.startTime = performance.now();
        this.isRunning = true;
        this.variables.dimPurchasesLeft = undefined;
        GameIntervals.restart();
        this.runNextPendingInstruction();
        return true;
    },

    reset(save=null) {
        this.importSave(save);
        this.startTime = null;
        this.pause();
        this.startTime = player.records.realTimePlayed;
        this.tickSwitch = true;
        this.instructions = [];
        this.currentInstruction = 0;
        this.variables.tick = 0;
        return true;
    },

    pause() {
        GameIntervals.stop();
        TAS.isRunning = false;
        return true;
    },

    export() {
        console.log("TAS finished running, exporting save:");
        GameStorage.save();
        TAS.exportedSave = GameStorage.exportModifiedSave();
        return console.log(TAS.exportedSave);
    }
};

// formatting these so the colons line up
export const actions = {
    ["buyOneDimension"]     :   buyOneDimension,
    ["buyDimension"]        :   buyDimension,
    ["buyTickSpeed"]        :   buyTickSpeed,
    ["buyDimensionBoost"]   :   buyDimensionBoost,
    ["buyGalaxyReset"]      :   requestGalaxyReset,

    ["trySacrificeReset"]   :   trySacrificeReset,
    ["tryBigCrunchReset"]   :   tryBigCrunchReset,

    ["getInstruction"]      :   TAS.getInstructions,
    ["loadInstruction"]     :   TAS.loadInstructions,

    ["wait"]                :   waitNextTick,
    ["startCycle"]          :   startCycle,
    ["endCycle"]            :   endCycle,

    ["dp"]                  :   simulateEvent,

    ["exportSave"]          :   exportSave,
    ["importSave"]          :   importSave
};
export const pathsToFiles = {
    'instructions': [
        "/input/instructions/0000 - start.json",
        "/input/instructions/0001 - over in 0 seconds.json",
        "/input/instructions/0002 - g0d0.json",
        "/input/instructions/0003 - g0d1.json",
        "/input/instructions/0004 - g0d2.json",
        "/input/instructions/0005 - g0d3.json",
        "/input/instructions/0006 - g0d4.json",
        "/input/instructions/0007 - g0d5.json",
        "/input/instructions/0008 - g0d6.json",
        "/input/instructions/0009 - g0d7.json",
        "/input/instructions/0010 - g0d8.json",
        "/input/instructions/0011 - g1d0.json",
        "/input/instructions/0012 - g1d1.json",
        "/input/instructions/0013 - g1d2.json",
        "/input/instructions/0014 - g1d3.json",
        "/input/instructions/0015 - g1d4.json",
        "/input/instructions/0016 - g1d5.json",
        "/input/instructions/0017 - g1d6.json",
        "/input/instructions/0018 - g1d7.json",
        "/input/instructions/0019 - g1d8.json",
        "/input/instructions/0020 - g1d9.json",
        "/input/instructions/0021 - g1d10.json",
        "/input/instructions/0022 - g1d11.json",
        "/input/instructions/0023 - g1d12.json",
        "/input/instructions/0024 - g2d0.json",
        "/input/instructions/0025 - g2d1.json",
        "/input/instructions/0026 - g2d2.json",
        "/input/instructions/0027 - g2d3.json",
        "/input/instructions/0028 - g2d4.json",
        "/input/instructions/0029 - g2d5.json",
        "/input/instructions/0030 - g2d6.json",
        "/input/instructions/0031 - g2d7.json",
        "/input/instructions/0032 - g2d8.json",
        "/input/instructions/0033 - g2d9.json",
        "/input/instructions/0034 - g2d10.json",
        "/input/instructions/0035 - g2d11.json",
        "/input/instructions/0036 - g2d12.json",
        "/input/instructions/0037 - g2d13.json",
        "/input/instructions/0038 - g2d14.json",
        "/input/instructions/0039 - g2d15.json",
        "/input/instructions/0040 - g2d16.json"
    ],
    'saves': [
        '/input/savestates/0000 - start.txt', 
        '/input/savestates/0001 - over in 0 seconds.txt', 
        '/input/savestates/0002 - g0d0.txt', 
        '/input/savestates/0003 - g0d1.txt', 
        '/input/savestates/0004 - g0d2.txt', 
        '/input/savestates/0005 - g0d3.txt', 
        '/input/savestates/0006 - g0d4.txt', 
        '/input/savestates/0007 - g0d5.txt', 
        '/input/savestates/0008 - g0d6.txt', 
        '/input/savestates/0009 - g0d7.txt', 
        '/input/savestates/0010 - g0d8.txt', 
        '/input/savestates/0011 - g1d0.txt', 
        '/input/savestates/0012 - g1d1.txt', 
        '/input/savestates/0013 - g1d2.txt', 
        '/input/savestates/0014 - g1d3.txt', 
        '/input/savestates/0015 - g1d4.txt', 
        '/input/savestates/0016 - g1d5.txt', 
        '/input/savestates/0017 - g1d6.txt', 
        '/input/savestates/0018 - g1d7.txt', 
        '/input/savestates/0019 - g1d8.txt', 
        '/input/savestates/0020 - g1d9.txt', 
        '/input/savestates/0021 - g1d10.txt', 
        '/input/savestates/0022 - g1d11.txt', 
        '/input/savestates/0023 - g1d12.txt', 
        '/input/savestates/0024 - g2d0.txt', 
        '/input/savestates/0025 - g2d1.txt', 
        '/input/savestates/0026 - g2d2.txt', 
        '/input/savestates/0027 - g2d3.txt', 
        '/input/savestates/0028 - g2d4.txt', 
        '/input/savestates/0029 - g2d5.txt', 
        '/input/savestates/0030 - g2d6.txt', 
        '/input/savestates/0031 - g2d7.txt', 
        '/input/savestates/0032 - g2d8.txt', 
        '/input/savestates/0033 - g2d9.txt', 
        '/input/savestates/0034 - g2d10.txt', 
        '/input/savestates/0035 - g2d11.txt', 
        '/input/savestates/0036 - g2d12.txt', 
        '/input/savestates/0037 - g2d13.txt', 
        '/input/savestates/0038 - g2d14.txt', 
        '/input/savestates/0039 - g2d15.txt', 
        '/input/savestates/0040 - g2d16.txt', 
        '/input/savestates/0041 - bigcrunch.txt'
    ]
};
export async function getSave(path=null) {
    if (!path) path = pathsToFiles.saves[0];

    const response = await fetch(path);
    if (!response.ok) {
        throw new Error(`Error fetching save at path: ${path}`);
    };

    return (await response.text()).trim();
};
export function tasTick() {
    TAS.variables.tick++;
    TAS.runNextPendingInstruction();
    return true;
};

export function waitNextTick() {
    return TAS.tickSwitch = !TAS.tickSwitch;
};

export function startCycle(repeat) {
    TAS.cycleCounter = repeat;
    TAS.lastCycleInstruction = TAS.currentInstruction;
    return true;
};

export function endCycle() {
    TAS.cycleCounter -= 1;
    if (TAS.cycleCounter > 0) {
        TAS.currentInstruction = TAS.lastCycleInstruction;
    }
    return true;
};

export function exportSave() {
    TAS.variables.save = GameStorage.exportModifiedSave();
    return true;
};

export function importSave() {
    GameStorage.import(TAS.variables.save);
    return true;
};

export function simulateEvent(type, key, keyCode) {
    const event = new KeyboardEvent(type, {
        key,
        keyCode: keyCode,
        which: keyCode
    });
    return document.dispatchEvent(event);
};

export function buyDimension(tier, times) {
    if (TAS.variables.dimPurchasesLeft === undefined) TAS.variables.dimPurchasesLeft = times;
    let successfulPurchase = true;
    while (successfulPurchase) {
        successfulPurchase = buyOneDimension(tier);
        if (successfulPurchase) {
            TAS.variables.dimPurchasesLeft--;
        }
        if (TAS.variables.dimPurchasesLeft <= 0) {
            TAS.variables.dimPurchasesLeft = undefined;
            return true;
        }
    }
    return false;
}

export function buyDimensionBoost() {
    let oldValue = player.dimensionBoosts;
    requestDimensionBoost();
    let newValue = player.dimensionBoosts;
    return oldValue !== newValue;
};

export function trySacrificeReset(value) {
    if (new Decimal(value).divideBy(Sacrifice.totalBoost).lte(Sacrifice.nextBoost)) {
        sacrificeReset();
        return true;
    }
    return false;
};

export function tryBigCrunchReset(ip) {
    if (!Player.canCrunch || gainedInfinityPoints().gte(Decimal.pow10(ip))) return false;
    bigCrunchReset(false, false);
    return true;
};