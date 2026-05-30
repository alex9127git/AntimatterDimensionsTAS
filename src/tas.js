import { buyOneDimension, buyTickSpeed, GameIntervals, GameSaveSerializer, GameStorage, requestDimensionBoost, requestGalaxyReset, sacrificeReset } from "./core/globals"

/*  Notes from Jade :3
*-- We can add a .bind(TAS) to the command line so we can continue using "this"
*-- .bind changes the "this" context for the function that .bind is called on
*-- example:
instructions.push(createInstruction(() => actions.bind(TAS)&&actions[fn](...args)));
*-- .bind() regardless of the argument passed in returns a truthy value
*-- it allows the next side of the and operator to evaluate, and those are also truthy
*-- this means that it will be return by the createInstructions, 
*-- note: this is bad practice, horrible readability, and shouldn't be used.
*-- this is just the idea in spirit.
*-- for an actual suggestion of code:

instructions.push(createInstruction(() => {
    const fn = actions[fn].bind(TAS);
    return fn(...args);
}));

*/






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
            return;
        };

        let instruction = this.instructions[index];
        return instruction.run();
    },

    async prepare(pathsToInstructions, pathToSave=null) {
        await TAS.reset(pathToSave);
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

    async importSave(path=null) {
        let save = "";
        const response = await fetch(path);
        save = await response.text();
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
        console.log("TAS started running");
        this.isRunning = true;
        GameIntervals.restart();
        this.runNextPendingInstruction();
        return true;
    },

    async reset(pathToSave=null) {
        await this.importSave(pathToSave);
        // use "this", unless we expect to call this
        // function from the command line.
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
        return console.log(GameStorage.exportModifiedSave());
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