#pragma once

#include <stddef.h>

// Build a JSON telemetry string into `buf`. Returns the number of bytes
// written (excluding the NUL), or 0 on error.
//
// Example output:
// {"uptime_s":42,"free_heap":210344,"min_free_heap":198765,
//  "cpu_load":[12.3,3.1],"rssi":-58,"led":"on"}
size_t metrics_build_json(char *buf, size_t buf_len);
