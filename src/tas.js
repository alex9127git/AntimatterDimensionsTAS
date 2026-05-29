import { buyOneDimension, buyTickSpeed, GameStorage, requestDimensionBoost, requestGalaxyReset, sacrificeReset } from "./core/globals"

export const actions = {
    ["buyOneDimension"]: buyOneDimension,
    ["buyTickSpeed"]: buyTickSpeed,
    ["buyDimensionBoost"]: buyDimensionBoost,
    ["trySacrificeReset"]: trySacrificeReset,
    ["wait"]: waitNextTick,
    ["dp"]: simulateEvent,
    ["galaxy"]: requestGalaxyReset
}

export const TAS = {
    isRunning: false,
    nextTickSwitch: true,

    instructions: [],

    currentInstruction: 0,
    startTime: null,
    queue: [],

    start() {
        console.log("TAS started running");
        // this.startTime = player.records.totalTimePlayed; ? is this necessary
        this.isRunning = true;
        this.nextTickSwitch = true;
        this.currentInstruction = 0;
        this.runNextPendingInstruction();
        return true;
    },

    runNextPendingInstruction() {
        if (!this.isRunning) return;
        let isSuccessful = true;
        while (isSuccessful) {
            isSuccessful = this.runOneInstruction(this.currentInstruction);
            if (isSuccessful) {
                this.startTime = player.records.totalTimePlayed
                console.log(`
                    Bought at: ${player.records.totalTimePlayed.toFixed(3)}ms,
                    step: ${player.records.totalTimePlayed.toFixed(3)}`);
	            this.currentInstruction += 1;
            };
        };
    },

    runOneInstruction(index) {
        if (index >= this.instructions.length) {
            TAS.isRunning = false;
            console.log("TAS finished running, exporting save:");
            GameStorage.save();
            return console.log(GameStorage.exportModifiedSave());
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
        TAS.start();
    },

    async importSave(path) {
        const response = await fetch(path);
        const text = await response.text();
        GameStorage.import(text);
        return true;
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
    }
};

export function tasTick() {
    TAS.runNextPendingInstruction();
    return true;
};

export function waitNextTick() {
    TAS.nextTickSwitch = !TAS.nextTickSwitch;
    return TAS.nextTickSwitch;
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