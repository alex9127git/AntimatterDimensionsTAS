import { buyOneDimension, buyTickSpeed, GameIntervals, GameSaveSerializer, GameStorage, requestDimensionBoost, requestGalaxyReset, sacrificeReset } from "./core/globals"

/*  Notes from Jade :3
--- :blob:
--- removed commented out code:
--- --- actions; TAS.pushSave.
--- added startTime for debugging
--- added timestamps in console.log for debugging
*/

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

    variables: {},
    intervals: {},

    runNextPendingInstruction() {
        if (!this.isRunning) return;
        let isSuccessful = true;
        while (isSuccessful) {
            isSuccessful = this.runOneInstruction(this.currentInstruction);
            if (isSuccessful) {
                if (TAS.variables.debug === 69) {
                    console.log(`
                        Instruction : ${TAS.currentInstruction}, 
                        cycleCounter: ${TAS.cycleCounter},
                        timestamp   :realtime: ${(performance.now() - this.startTime).toFixed(0)},
                                     gametime: ${player.records.totalTimePlayed}
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

    async getSave(path=null) {
        let save = "";
        const response = await fetch(path);
        save = await response.text();
        save = save.trim();
        return save
    },

    importSave(save) {
        // note to Jade:
        // check player object checks if there are any problems with the save, if there is none
        // returns an empty string.
        if (GameStorage.checkPlayerObject(GameSaveSerializer.deserialize(save)) === "") {
            console.log("Save found, importing");
            GameStorage.import(save);
            return true;
        } else {
            console.log("Path to save doesn't exist or save is invalid, hard resetting");
            dev.hardReset();
            Speedrun.prepareSave();
        }
    },

    async getInstructions(path) {
        const response = await fetch(path);
        const text = await response.text();
        const commands = JSON.parse(text);
        const instructions = [];

        commands.forEach(([fn, args]) => {
            instructions.push(createInstruction(() => actions[fn](...args)));
        });   
        TAS.queue = instructions;
        return true;
    },

    loadInstructions() {
        TAS.instructions.push(...TAS.queue);
        TAS.queue = [];
        return true;
    },

    start() {
        this.startTime = performance.now();
        console.log("TAS started running");
        this.isRunning = true;
        GameIntervals.restart();
        this.runNextPendingInstruction();
        return true;
    },

    reset(save=null) {
        this.importSave(save);
        this.startTime = null;
        this.pause();
        this.startTime = player.records.totalTimePlayed;
        this.tickSwitch = true;
        this.instructions = [];
        this.currentInstruction = 0;
        return true;
    },

    pause() {
        // use "TAS" to use this from the command line
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
    ["buyTickSpeed"]        :   buyTickSpeed,
    ["buyDimensionBoost"]   :   buyDimensionBoost,
    ["trySacrificeReset"]   :   trySacrificeReset,
    ["buyGalaxyReset"]      :   requestGalaxyReset,
    ["getInstruction"]      :   TAS.getInstructions,
    ["loadInstruction"]     :   TAS.loadInstructions,
    ["wait"]                :   waitNextTick,
    ["dp"]                  :   simulateEvent,
    ["startCycle"]          :   startCycle,
    ["endCycle"]            :   endCycle,
    ["exportSave"]          :   exportSave,
    ["importSave"]          :   importSave
};

export function createInstruction(action) {
    return {
        action: action,
        run: (() => action())
    };
};

export function tasTick() {
    TAS.runNextPendingInstruction();
    return true;
};

export function waitNextTick() {
    TAS.tickSwitch = !TAS.tickSwitch;
    return TAS.tickSwitch;
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

export function buyDimensionBoost() {
    let oldValue = player.dimensionBoosts;
    requestDimensionBoost();
    let newValue = player.dimensionBoosts;
    return oldValue !== newValue;
};

export function trySacrificeReset(value) {
    if (Sacrifice.nextBoost.gte(value)) {
        sacrificeReset();
        return true;
    }
    return false;
};