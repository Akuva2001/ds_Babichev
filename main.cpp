#include "DSSimul.h"
//#include <unistd.h>
#include <assert.h>
#include <sstream>

int workFunction_Bully(Process *dp, Message m)
{
    string s = m.getString();
    NetworkLayer *nl = dp->networkLayer;
    if (!dp->isMyMessage("Bully", s)) return false;
    set<int> neibs = dp->neibs();
    if (s == "Bully_Election" || s == "Bully_Election_Init") {
        printf("%3d: from %d: Start Election\n", dp->node, m.from);
        if (s != "Bully_Election_Init"){
            nl->send(dp->node, m.from, Message("Bully_Alive"));
            printf("%3d: sent to %3d Alive\n", dp->node, m.from);
        }
        if (dp->contextBully.started) {
            printf("%3d: from %d: Election already started, do nothing\n", dp->node, m.from);
            return true;
        }
        dp->contextBully.started = true;
        dp->contextBully.start_time = dp->networkLayer->tick;
        if (m.from < dp->node) {
            for (auto n: neibs) {
                if (n > dp->node) {
                    nl->send(dp->node, n, Message("Bully_Election"));
                    printf("%3d: sent to %3d Election\n", dp->node, n);
                }
            }
        }
    } else if (s == "Bully_Alive") {
        dp->contextBully.finished = true;
        printf("%3d: Bully_Alive by %3d\n", dp->node, m.from);
    } else if (s == "*TIME"){
        if (dp->contextBully.finished || !dp->contextBully.started) return true;
        printf("%3d: TIME, tick: %5lld\n", dp->node, dp->networkLayer->tick);

        if (dp->contextBully.started && !dp->contextBully.finished &&
            dp->networkLayer->tick >= dp->contextBully.start_time + dp->contextBully.wait_delta)
        {
            dp->contextBully.finished = true;
            for (auto n: neibs) {
                nl->send(dp->node, n, Message("Bully_Coordinator"));
            }
            printf("%3d: I'm Coordinator\n", dp->node);
            dp->contextBully.coordinator = dp->node;
        }
    } else if (s == "Bully_Coordinator") {
        printf("%3d: %3d is Coordinator\n", dp->node, m.from);
        dp->contextBully.coordinator = m.from;
        dp->contextBully.finished = true;
    } else {
        printf("%3d: Something strange#1 by %3d\n", dp->node, m.from);
    }

    return true;
}


int main(int argc, char **argv)
{
    string configFile = argc > 1 ? argv[1] : "config.data";
    World w; 
    w.registerWorkFunction("Bully", workFunction_Bully);
    if (w.parseConfig(configFile)) {
        this_thread::sleep_for(chrono::milliseconds(3000000));
	} else {
        printf("can't open file '%s'\n", configFile.c_str());
    }
}

