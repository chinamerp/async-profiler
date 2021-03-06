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

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "arguments.h"


// Parses agent arguments.
// The format of the string is:
//     arg[,arg...]
// where arg is one of the following options:
//     start         - start profiling
//     stop          - stop profiling
//     status        - print profiling status (inactive / running for X seconds)
//     cpu           - profile CPU (default)
//     heap          - profile heap allocations
//     collapsed[=C] - dump collapsed stacks (the format used by FlameGraph script)
//                     C is counter type: 'samples' or 'total'
//     folded[=C]    - synonym for collapsed
//     summary       - dump profiling summary (number of collected samples of each type)
//     traces[=N]    - dump top N call traces
//     flat[=N]      - dump top N methods (aka flat profile)
//     interval=N    - sampling interval in ns (default: 1'000'000, i.e. 1 ms)
//     framebuf=N    - size of the buffer for stack frames (default: 1'000'000)
//     file=FILENAME - output file name for dumping
//
// It is possible to specify multiple dump options at the same time

const char* Arguments::parse(char* args) {
    if (strlen(args) >= sizeof(_buf)) {
        return "Argument list too long";
    }
    strcpy(_buf, args);

    for (char* arg = strtok(_buf, ","); arg != NULL; arg = strtok(NULL, ",")) {
        char* value = strchr(arg, '=');
        if (value != NULL) *value++ = 0;

        if (strcmp(arg, "start") == 0) {
            _action = ACTION_START;
        } else if (strcmp(arg, "stop") == 0) {
            _action = ACTION_STOP;
        } else if (strcmp(arg, "status") == 0) {
            _action = ACTION_STATUS;
        } else if (strncmp(arg, "cpu", 3) == 0) {
            _mode = MODE_CPU;
            char* event = strchr(arg, ':');
            if (event != NULL) {
                event = event + 1;
                if (strcmp(event, "cycles") == 0) {
                    _event_type = EVENT_TYPE_CYCLES;
                } else if (strcmp(event, "context-switches") == 0) {
                    _event_type = EVENT_TYPE_CTX_SWITCHES;
                } else if (strcmp(event, "branch-misses") == 0) {
                    _event_type = EVENT_TYPE_BRANCH_MISSES;
                } else if (strcmp(event, "cache-misses") == 0) {
                    _event_type = EVENT_TYPE_CACHE_MISSES;
                } else if (strcmp(event, "L1-dcache-load-misses") == 0) {
                    _event_type = EVENT_TYPE_L1D_LOAD_MISSES;
                } else if (strcmp(event, "LLC-load-misses") == 0) {
                    _event_type = EVENT_TYPE_LLC_LOAD_MISSES;
                }
                else {
                    return "unknown event type";
                }
            }
        } else if (strcmp(arg, "heap") == 0) {
            _mode = MODE_HEAP;
        } else if (strcmp(arg, "collapsed") == 0 || strcmp(arg, "folded") == 0) {
            _action = ACTION_DUMP;
            _dump_collapsed = true;
            _counter = value == NULL || strcmp(value, "samples") == 0 ? COUNTER_SAMPLES : COUNTER_TOTAL;
        } else if (strcmp(arg, "summary") == 0) {
            _action = ACTION_DUMP;
            _dump_summary = true;
        } else if (strcmp(arg, "traces") == 0) {
            _action = ACTION_DUMP;
            _dump_traces = value == NULL ? INT_MAX : atoi(value);
        } else if (strcmp(arg, "flat") == 0) {
            _action = ACTION_DUMP;
            _dump_flat = value == NULL ? INT_MAX : atoi(value);
        } else if (strcmp(arg, "interval") == 0) {
            if (value == NULL || (_interval = atoi(value)) <= 0) {
                return "interval must be > 0";
            }
        } else if (strcmp(arg, "framebuf") == 0) {
            if (value == NULL || (_framebuf = atoi(value)) <= 0) {
                return "framebuf must be > 0";
            }
        } else if (strcmp(arg, "file") == 0) {
            if (value == NULL || value[0] == 0) {
                return "file must not be empty";
            }
            _file = value;
        }
    }

    return NULL;
}
