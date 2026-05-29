import { buyOneDimension, buyTickSpeed, GameIntervals, GameStorage, requestDimensionBoost, requestGalaxyReset, sacrificeReset } from "./core/globals"

export const TAS = {
    isRunning: false,
    tickSwitch: true,

    startTime: null,
    instructions: [],
    currentInstruction: 0,
    queue: [],

    runNextPendingInstruction() {
        if (!this.isRunning) return;
        let isSuccessful = true;
        while (isSuccessful) {
            isSuccessful = this.runOneInstruction(this.currentInstruction);
            if (isSuccessful) {
                this.startTime = this.startTime || performance.now();
                if (this.currentInstruction > 20) {
                    console.log(`
                        Bought at: ${performance.now() - this.startTime}ms,
                        step: ${this.currentInstruction}`);
                };
	            this.currentInstruction += 1;
            };
        };
    },

    runOneInstruction(index) {
        if (index >= this.instructions.length) {
            this.pause();
            this.export();
        };

        let instruction = this.instructions[index];
        return instruction.run();
    },

    async prepareAndStart(pathsToInstructions, pathToSave=null) {
        await pathsToInstructions.forEach(async (path) => {
            await this.getInstructions(path);
            this.loadInstructions();
        });
        console.log("Finished loading instructions");
        if (pathToSave === null) {
            console.log("Save not found, hard resetting");
            dev.hardReset();
            Speedrun.prepareSave("JadeGPTas");
        } else {
            console.log("Save found, importing");
            await this.importSave(pathToSave);
        }
        TAS.reset();
        TAS.start();
    },

    async importSave(path=null) {
        let save = "";
        try {
            const response = await fetch(path);
            save = await response.text();
        } catch(e) {
            console.log(e);
        }
        if (!save) {
            console.log("Save not found, hard resetting");
            dev.hardReset();
            Speedrun.prepareSave();
            return true;
        } else {
            try {
                GameStorage.import(save);
                return true;
            } catch(e) {
                console.log("Save is malformed");
                return false;
            }
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
        this.queue = instructions;
        return true;
    },

    loadInstructions() {
        this.instructions.push(...this.queue);
        return true;
    },

    start() {
        console.log("TAS started running");
        this.isRunning = true;
        GameIntervals.restart();
        this.runNextPendingInstruction();
        return true;
    },

    reset(pathToSave=null) {
        this.importSave(pathToSave);
        TAS.pause();
        this.startTime = player.records.totalTimePlayed;
        this.tickSwitch = true;
        this.currentInstruction = 0;
        return true;
    },

    pause() {
        GameIntervals.stop();
        this.isRunning = false;
        return true;
    },

    export() {
        GameStorage.save();
        console.log(GameStorage.exportModifiedSave());
        return true;
    },

    restart(name = "JadeGPTas") {
        TAS.currentInstruction = 0;
        TAS.isRunning = false;
        TAS.startTime = null;
        dev.hardReset();
        Speedrun.prepareSave(name);
    }
};

export const actions = {
    ["buyOneDimension"]: buyOneDimension,
    ["buyTickSpeed"]: buyTickSpeed,
    ["buyDimensionBoost"]: buyDimensionBoost,
    ["trySacrificeReset"]: trySacrificeReset,
    ["buyGalaxyReset"]: requestGalaxyReset,
    ["getInstruction"]: TAS.getInstructions,
    ["loadInstruction"]: TAS.loadInstructions,
    ["wait"]: waitNextTick,
    ["dp"]: simulateEvent
};

export function tasTick() {
    TAS.runNextPendingInstruction();
    return true;
};

export function waitNextTick() {
    TAS.tickSwitch = !TAS.tickSwitch;
    return TAS.tickSwitch;
};

export function createInstruction(action) {
    return {
        action: action,
        run: (() => action())
    };
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
}

export function trySacrificeReset(value) {
    if (Sacrifice.nextBoost.gte(value)) {
        sacrificeReset();
        return true;
    }
    return false;
}