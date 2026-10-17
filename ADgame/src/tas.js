import { bigCrunchReset, buyOneDimension, buyTickSpeed, GameIntervals, GameSaveSerializer, GameStorage, requestDimensionBoost, requestGalaxyReset, sacrificeReset } from "./core/globals"
import { gainedInfinityPoints } from "./game";
import { pathsToFiles } from "./pathsOfFiles";

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
                if (TAS.variables.debug & 1 === 1) dumpLog();

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

    async prepare(pathsToInstructions, save) {
        save ??= pathsToFiles.saves[0];

        if (save?.includes('/input')) {
            save = await getSave(save);
        };

        TAS.reset(save);

        let i = 1,
            max = pathsToInstructions.length;

        for (const path of pathsToInstructions) {
            console.log("Loading instructions (" + i + "/" + max + ")");
            await this.getInstructions(path);
            this.loadInstructions();
            i++;
        }
        console.log("Finished loading instructions");
    },

    importSave(save) {
        if (save?.includes('AntimatterDimensions')) {
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
        try {
            const response = await fetch(path);
            if (!response.ok) {
                throw new Error(`Error fetching save from ${path}. ${response.status}`);
            };
            const commands = await response.json();
            TAS.queue = commands.map(([fn, args]) => ({
                fn,
                args,
                action: actions[fn],
                run() {
                    return this.action(...this.args);
                }
            }));
            return true;
        } catch(e) {
            throw new Error(`Error parsing path to save: ${path}`, {cause: e});
        };
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
    },

    get nextBoost() {
        let amount = AntimatterDimension(1).amount;
        let sacrificed = player.sacrificed.clampMin(1);
        let unweighted = amount.add(sacrificed);
        let weighted = new Decimal((unweighted.log10() / 10) / Math.max(sacrificed.log10() / 10, 1));
        return weighted.clampMin(1).pow(Sacrifice.sacrificeExponent);
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

export async function getSave(path=null) {
    if (!path) path = pathsToFiles.saves[0];

    const response = await fetch(path);
    if (!response.ok) {
        throw new Error(`Error fetching save at path: ${path}`);
    };

    return (await response.text()).trim();
};

export function dumpLog() {
    console.log(`
        Instruction  : ${TAS.currentInstruction}, 
        cycleCounter : ${TAS.cycleCounter},
        tick         : ${TAS.variables.tick},
        realtime     : ${(performance.now() - TAS.startTime).toFixed(0)},
        gametime     : ${player.records.realTimePlayed}
    `);
}
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
    if (new Decimal(value).divideBy(Sacrifice.totalBoost).lte(TAS.nextBoost)) {
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