/*
 * Copyright 2017 Andrei Pangin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef __APPLE__

#include <sys/time.h>
#include "perfEvents.h"
#include "profiler.h"


int PerfEvents::_max_events;
PerfEvent* PerfEvents::_events;
int PerfEvents::_interval;


void PerfEvents::init() {}

int PerfEvents::tid() { return 0; }

void PerfEvents::createForThread(int tid)  {}
void PerfEvents::createForAllThreads()     {}
void PerfEvents::destroyForThread(int tid) {}
void PerfEvents::destroyForAllThreads()    {}


void PerfEvents::installSignalHandler() {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = NULL;
    sa.sa_sigaction = signalHandler;
    sa.sa_flags = SA_RESTART | SA_SIGINFO;

    sigaction(SIGPROF, &sa, NULL);
}

void PerfEvents::signalHandler(int signo, siginfo_t* siginfo, void* ucontext) {
    Profiler::_instance.recordSample(ucontext, _interval, 0, NULL);
}

bool PerfEvents::start(int interval, EventType type) {
    if (interval <= 0 || type != EVENT_TYPE_CPU_CLOCK) return false;
    _interval = interval;

    installSignalHandler();

    int sec = interval / 1000000000;
    int usec = (interval % 1000000000) / 1000;
    struct itimerval tv = {{sec, usec}, {sec, usec}};
    setitimer(ITIMER_PROF, &tv, NULL);

    return true;
}

void PerfEvents::stop() {
    struct itimerval tv = {{0, 0}, {0, 0}};
    setitimer(ITIMER_PROF, &tv, NULL);
}

int PerfEvents::getCallChain(const void** callchain, int max_depth) {
    return 0;
}

#endif // __APPLE__
