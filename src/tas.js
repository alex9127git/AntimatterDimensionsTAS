import { buyOneDimension } from "./core/globals"

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

    enable() {
        console.log("TAS started running");
	this.startTime = performance.now();
        this.isRunning = true;
        this.nextTickSwitch = true;
        this.currentInstruction = 0;
        this.runNextPendingInstruction();
    },

    waitNextTick() {
        this.nextTickSwitch = !this.nextTickSwitch;
        return this.nextTickSwitch;
    },

    runNextPendingInstruction() {
        if (!this.isRunning) return;
        let isSuccessful = true;
        while (isSuccessful) {
            isSuccessful = this.runOneInstruction(this.currentInstruction);
            if (isSuccessful && (TAS.currentInstruction > 22 || TAS.currentInstruction === 20)) {
		console.log(`
			Bought at: ${performance.now() - this.startTime},
			step: ${this.currentInstruction}`);
		this.currentInstruction += 1;
	    }
        }
    },

    runOneInstruction(index) {
        if (index >= this.instructions.length) {
            TAS.isRunning = false;
            console.log("TAS finished running");
            return;
        }
        let instruction = this.instructions[index];
        return instruction.run();
    },
    restart(name = "JadeGPTas") {
    	TAS.isRunning = false;
    	TAS.currentInstruction = 0;
    	dev.hardReset();
    	Speedrun.prepareSave(name);
    }
}

export function tasTick() {
    TAS.runNextPendingInstruction();
}

export function waitNextTick() {
    TAS.nextTickSwitch = !TAS.nextTickSwitch;
    return TAS.nextTickSwitch;
}

export function createInstruction(action) {
    return {
        action: action,
        run: (() => action())
    };
}

export function simulateEvent(type, key, keyCode) {
    const event = new KeyboardEvent(type, {
        key,
        keyCode: keyCode,
        which: keyCode
    });
    return document.dispatchEvent(event);
}