import { buyOneDimension, buyTickSpeed, requestDimensionBoost } from "./core/globals"

/*  Addition notes (from Jade :3):
*-- We can add a return true for functions that don't 
*-- have an implicit return statement,
*
*-- Cleaned up some of the logic, mainly:
*-- -- logic regarding creating the instructions
*-- -- especially as they are stored.
*-- -- See runNextPendingInstruction;
*
*-- The current problem is that
*-- -- requestDimensionBoost();
*-- -- doesn't return false/true, only undefined.
*-- -- this is annoying, we can either just force 
*-- -- it to return a boolean, or change our checks.
*
*/
export const actions = {
    ["buyOneDimension"]: buyOneDimension,
    ["buyTickSpeed"]: buyTickSpeed,
    ["requestDimensionBoost"]: requestDimensionBoost,
    ["wait"]: waitNextTick,
    ["dp"]: simulateEvent
}
export const TAS = {
    isRunning: false,
    nextTickSwitch: true,

    instructions: [
        createInstruction(() => simulateEvent("keydown", "up", 38)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "up", 38)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "down", 40)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "down", 40)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "left", 37)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "right", 39)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "left", 37)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "right", 39)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "b", 66)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => simulateEvent("keydown", "a", 65)),
        createInstruction(() => waitNextTick()),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => simulateEvent("keydown", "enter", 13)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(1)),
        createInstruction(() => buyOneDimension(2))
    ],

    currentInstruction: 0,
    startTime: null,
    queue: [],

    enable() {
        console.log("TAS started running");
        this.isRunning = true;
        this.nextTickSwitch = true;
        this.currentInstruction = 0;
        this.runNextPendingInstruction();
        return true;
    },

    // I'm scared to add a return 
    // statement on this behemoth of a function
    runNextPendingInstruction() {
        if (!this.isRunning) return;
        let isSuccessful = true;
        while (isSuccessful) {
            isSuccessful = this.runOneInstruction(this.currentInstruction);

            if (isSuccessful) {
                if (this.currentInstruction === 21 || this.currentInstruction > 22) {

                this.startTime = this.startTime || performance.now();
    
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
            TAS.isRunning = false;
        // we can just return the console.log statement,
        // this is still fine for readability.
            return console.log("TAS finished running");;
        };
        
        let instruction = this.instructions[index];
        return instruction.run();
    },

    restart(name = "JadeGPTas") {

        // reset the TAS values.
        TAS.startTime = null;
    	TAS.isRunning = false;
    	TAS.currentInstruction = 0;

    	dev.hardReset(); // reset the achievements
    	Speedrun.prepareSave(name); // enable speedrun mode
        return true;
    },

    async getInstructions(path) {
        const response = await fetch(path);
        const text = await response.text();
        const data = JSON.parse(text);
        const instructions = [];

        data.forEach(([fn, args]) => {
            instructions.push(createInstruction(() => actions[fn](...args)));
        });   
        this.queue = instructions;
        return true;
    },

    loadInstructions() {
        this.instructions = this.queue;
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