#pragma once
#include "ioutils.h"
#include "timer.h"
#include <iostream>


class Stats {
    private:
        Timer timer;
        Timer branchTimer;
        Timer checkTimer;
        Timer tickTimer;
        Timer purgeTimer;
        double branchTime = 0;
        double checkTime = 0;
        double tickTime = 0;
        double purgeTime = 0;

    public:
        void resetBranchTimer() {branchTimer.reset();}
        void resetCheckTimer() {checkTimer.reset();}
        void resetTickTimer() {tickTimer.reset();}
        void resetPurgeTimer() {purgeTimer.reset();}
        void updateBranchTime() {branchTime += branchTimer.reset();}
        void updateCheckTime() {checkTime += checkTimer.reset();}
        void updateTickTime() {tickTime += tickTimer.reset();}
        void updatePurgeTime() {purgeTime += purgeTimer.reset();}
        double getTimeRunning() const {return timer.peek();}
        void printStats(int maxStates) const {
            std::cout << std::endl;
            std::cout << "Finished! Total time elapsed: " << renderTime(timer.peek()) << std::endl;
            std::cout << "Max States: " << maxStates << std::endl;
            std::cout << "branch: " << renderTimeVerbose(branchTime) << std::endl;
            std::cout << "check: " << renderTimeVerbose(checkTime) << std::endl;
            std::cout << "tick: " << renderTimeVerbose(tickTime) << std::endl;
            std::cout << "purge: " << renderTimeVerbose(purgeTime) << std::endl;
        }
};
