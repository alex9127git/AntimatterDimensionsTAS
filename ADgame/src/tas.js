import { bigCrunchReset, buyOneDimension, buyTickSpeed, GameIntervals, GameStorage, requestDimensionBoost, requestGalaxyReset, sacrificeReset } from "./core/globals"
import { gainedInfinityPoints } from "./game";
import { pathsToFiles } from "./pathsOfFiles";
import { dispatch } from "./tasUI/injectUI";

export const TAS = {
    // superficial check; don't run while running.
    isRunning: false,
    // in 'wait' instructions, it prevents the TAS from performing an action for one tick.
    // it is switched from 'true' and 'false' to act as a switch.
    tickSwitch: true,

    // instructions act on a queue system.
    // track current instruction, perform, increment instruction.
    instructions: [],
    currentInstruction: 0,

    // cycles consolidate code by performing the same set of actions 
    // a certain amount of times, as defined in cycle counter
    lastCycleInstruction: 0, // this is the instruction number at startCycle for a loop
    cycleCounter: 0,

    // queue takes instructions for later. Flush it to add it to the instructions and clear it.
    queue: [],
    // importing/exporting save for the TAS to spam
    exportedSave: null,
    // in segmented runs, this holds the saves as-a-string.
    segments: [],

    // start time is purely diagnostic
    startTime: null,
    /*
     --- debug binary bitmask ---
     debug = -1 for all
     debug & 1 === logging
     debug & 2 === write output to ui
    */
    variables: {
        debug: 0,
        tick: 0
    },
    // tick counts the amount of ticks that has passed since start.
    
    // Only used for segment interval currently.
    // hold the references (and id) to different intervals to clear them.
    intervals: {},

    runNextPendingInstruction() {
        if (!this.isRunning) {
            return;
        }

        let isSuccessful = true;

        while (isSuccessful) {
            // perform the same action over and over until it is successful
            // this requires actions to explicitly return true/false in some way.
            // Have to be careful about truthy/falsey values.
            isSuccessful = this.runOneInstruction(this.currentInstruction);

            if (isSuccessful) {
                if (TAS.variables.debug & 1) {
                    dumpLog();
                }

	            this.currentInstruction += 1;
            }
        }
    },

    runOneInstruction(index) {
        // if it is at the end of the instruction set, clear and return.
        if (index >= this.instructions.length) {
            this.pause();
            this.export();
            return;
        }

        // then let the instruction run itself, returning its return value
        let instruction = this.instructions[index];

        return instruction.run();
    },

    getSegments(pathsToInstructions, startSave) {
        // clear the segments.
        TAS.segments = [];
        // then push the starting save. 
        // Starting save is likely going to be the path to a save.
        TAS.segments.push(startSave);

        let i = 0;
        TAS.intervals["segmentInterval"] = setInterval(async () => {
            if (!TAS.isRunning) {
                console.log(i);

                // at the end of the pathsToInstructions
                if (i >= pathsToInstructions.length) {
                    // push the current save
                    // this logs the save to the console also.
                    TAS.segments.push(GameStorage.exportModifiedSave());
                    // declare it's ended
                    console.log('TAS ended');
                    // then remove the segmentInterval
                    this.removeInterval("segmentInterval");

                    return;
                }
                // In cases where there is an exportedSave, push it into the segment
                if (TAS.exportedSave !== null) {
                    TAS.segments.push(TAS.exportedSave);
                }
                // dispatch UI updates if the bitflag is set
                if (TAS.variables.debug & 2) {
                    dispatch('inst3', pathsToInstructions[i].replace('/input/instructions/', ''));
                }
                // changes to prepareAndStart directly affect getSegments.
                await TAS.prepareAndStart([pathsToInstructions[i]], TAS.segments[i]);
                i++;
            }
        // every 2 seconds, it checks to see if it has ended, or if it requires rescheduling.
        }, 2000);
    },

    // the id is not the 
    removeInterval(id) {
        if (id in TAS.intervals) {
            clearInterval(TAS.intervals[id]);
            delete TAS.intervals[id];
            return true;
        }

        console.log(`ID: ${id} is not registered in segmentIntervals.`);

        return false;
    },

    // start the TAS directly after preparing.
    async prepareAndStart(pathsToInstructions, pathToSave=null) {
        await TAS.prepare(pathsToInstructions, pathToSave);

        TAS.start();
    },

    // this resets the current position to the state the save is in
    // and loads the queue into the instructions.
    async prepare(pathsToInstructions, save) {
        save ??= pathsToFiles.saves[0];

        if (save?.includes('\\input')) {
            save = await getSave(save);
        }

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
                // basic check to see if it contains a known string
        if (save?.includes('AntimatterDimensions')) {

            console.log("Save found, importing");

            GameStorage.import(save);

            return true;
        } else {
            console.log("Save String is invalid, hard resetting");
            // hard reset and prepare save doesn't have full overlap, 
            // so use both.
            dev.hardReset();
            Speedrun.prepareSave();

            return true;
        }
    },


    async getInstructions(path) {
        // this is more touchy, so a try and catch statement is safer.
        try {
            const response = await fetch(path);

            if (!response.ok) {
                throw new Error(`Error fetching save from ${path}. ${response.status}`);
            }

            // commands is a JSON file.
            const commands = await response.json();
            // fn, args, and action are for diagnostics.

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
        // push the instruction set, then flush the queue
        TAS.instructions.push(...TAS.queue);

        TAS.queue = [];

        return true;
    },

    start() {
        console.log("TAS started running");
        this.startTime = Date.now();
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

// actions are performed by TAS instructions
// with arguments passed in.
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

    if (!path) {
        // default path
        path = pathsToFiles.saves[0];
    }

    const response = await fetch(path);

    if (!response.ok) {
        throw new Error(`Error fetching save at path: ${path}`);
    }
        // had bugs where the text had white space surrounding it.
    return (await response.text()).trim();
};

// dumps the meta data into the console
export function dumpLog() {
    console.log(`
        Instruction  : ${TAS.currentInstruction}, 
        cycleCounter : ${TAS.cycleCounter},
        tick         : ${TAS.variables.tick},
        realtime     : ${(Date.now() - TAS.startTime).toFixed(0)},
        gametime     : ${player.records.realTimePlayed}
    `);
}
// the UI using the dispatched events to update itself.
function dispatch_events() {
    if (TAS.variables.debug & 2) {
        dispatch('cycle', TAS.cycleCounter ?? 0);
        dispatch('inst1', TAS.currentInstruction);
        dispatch('inst2', TAS.instructions[TAS.currentInstruction]?.fn + TAS.instructions[TAS.currentInstruction]?.args?.join(' '));
        dispatch('gala1', player.galaxies);
        dispatch('dimb1', player.dimensionBoosts);
        dispatch('time1', new Date(player.records.realTimePlayed).toISOString().slice(11,23));
        dispatch('time2', new Date(Date.now() - TAS.startTime).toISOString().slice(11, 23));
    }
}
export function tasTick() {
    TAS.variables.tick++;

    dispatch_events();

    TAS.runNextPendingInstruction();

    return true;
};

export function waitNextTick() {
    // simple flip
    return TAS.tickSwitch = !TAS.tickSwitch;
};

export function startCycle(count) {
    TAS.cycleCounter = count;

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

// Different platforms require different values to be accept.
// So all three are required.
export function simulateEvent(type, key, keyCode) {
    const event = new KeyboardEvent(type, {
        key,
        keyCode: keyCode,
        which: keyCode
    });

    return document.dispatchEvent(event);
};



export function buyDimension(tier, times) {
    if (TAS.variables.dimPurchasesLeft === undefined) {
        TAS.variables.dimPurchasesLeft = times;
    }

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
    // requestDimensionsBoost does not return true/false
    // on successful purchase, so you just have to check if it's the same amount before and after.
    let oldValue = player.dimensionBoosts;

    requestDimensionBoost();

    let newValue = player.dimensionBoosts;

    return oldValue !== newValue;
};

export function trySacrificeReset(value) {
    // if the (value / total boost) <= next boost, it's safe to sacrifice
    if (new Decimal(value).divideBy(Sacrifice.totalBoost).lte(TAS.nextBoost)) {
        sacrificeReset();
        return true;
    }

    return false;
};

export function tryBigCrunchReset(goal) {
    // if the player can't crunch or the amount you would gain on crunching
    // is less than 10 ^ goal, don't crunch
    if (!Player.canCrunch || gainedInfinityPoints().lt(Decimal.pow10(goal))) {
        return false;
    }

    bigCrunchReset(false, false);

    return true;
};