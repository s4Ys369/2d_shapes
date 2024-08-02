
/*
* This file includes code from the Tiny3D library.
* Tiny3D is licensed under the MIT License.
* See the LICENSES directory for the full text of the MIT License.
*
* Original code by Max Bebök 
* Adapted by s4ys
* August 2024
*
* Description of changes or adaptations made:
* - Removed graphical rendering code
* - Focused on profiling data printing
*
*
* Original source: https://github.com/HailToDodongo/tiny3d/blob/main/examples/99_testscene/debug_overlay.h
*/

#include "rspq_constants.h"
#if defined(RSPQ_PROFILE) && RSPQ_PROFILE
#ifndef PROFILE_PRINT_H
#define PROFILE_PRINT_H

#include <libdragon.h>
#include "rspq_profile.h"

typedef struct {
    uint32_t calls;
    int32_t timeUs;
    uint32_t index;
    const char *name;
    bool isIdle;
} ProfileSlot;

#define RCP_TICKS_TO_USECS(ticks) (((ticks) * 1000000ULL) / RCP_FREQUENCY)

static rspq_profile_data_t profile_data;

static void debug_profile_patch_name(ProfileSlot *slot) {
    switch (slot->index) {
        case 0: slot->name = "builtins"; break;
        case RSPQ_PROFILE_CSLOT_WAIT_CPU: slot->name = "CPU"; break;
        case RSPQ_PROFILE_CSLOT_WAIT_RDP: slot->name = "RDP"; break;
        case RSPQ_PROFILE_CSLOT_WAIT_RDP_SYNCFULL: slot->name = "SYNC_FULL"; break;
        case RSPQ_PROFILE_CSLOT_WAIT_RDP_SYNCFULL_MULTI: slot->name = "multi SYNC_F"; break;
        case RSPQ_PROFILE_CSLOT_OVL_SWITCH: slot->name = "Ovl switch"; break;
        default: slot->name = "Unknown"; break;
    }
}

static bool debug_profile_is_idle(uint32_t index) {
    return index == RSPQ_PROFILE_CSLOT_WAIT_CPU || index == RSPQ_PROFILE_CSLOT_WAIT_RDP
        || index == RSPQ_PROFILE_CSLOT_WAIT_RDP_SYNCFULL || index == RSPQ_PROFILE_CSLOT_WAIT_RDP_SYNCFULL_MULTI;
}

static int debug_profile_slot_compare(const void *a, const void *b) {
    const ProfileSlot *slotA = (const ProfileSlot *)a;
    const ProfileSlot *slotB = (const ProfileSlot *)b;
    int timeDiff = slotB->timeUs - slotA->timeUs;
    return timeDiff == 0 ? (slotB->calls - slotA->calls) : timeDiff;
}

void debug_print_profile_data() {
    if (profile_data.frame_count == 0) return;

    uint64_t totalTicks = 0;
    uint32_t timeTotalBusy = 0;
    uint32_t timeTotalWait = 0;

    // Copy & convert performance data to a sortable array
    const uint32_t SLOT_COUNT = RSPQ_PROFILE_SLOT_COUNT + 1;
    ProfileSlot slots[SLOT_COUNT];
    for (size_t i = 0; i < RSPQ_PROFILE_SLOT_COUNT; i++) {
        ProfileSlot *slot = &slots[i];
        slot->index = i;
        slot->isIdle = debug_profile_is_idle(i);
        slot->name = profile_data.slots[i].name;

        if (slot->name == NULL) continue;
        debug_profile_patch_name(slot);

        totalTicks += profile_data.slots[i].total_ticks;

        slot->calls = (uint32_t)(profile_data.slots[i].sample_count / profile_data.frame_count);
        slot->timeUs = RCP_TICKS_TO_USECS(profile_data.slots[i].total_ticks / profile_data.frame_count);

        if (slot->isIdle) {
            timeTotalWait += slot->timeUs;
        } else {
            timeTotalBusy += slot->timeUs;
        }
    }

    qsort(slots, RSPQ_PROFILE_SLOT_COUNT, sizeof(ProfileSlot), debug_profile_slot_compare);

    debugf("\nRSP Queue profiling (averages per frame over %llu frames):\n", profile_data.frame_count);
    debugf("  %-20s %5s %7s\n", "Type", "Calls", "Time (us)");
    for (size_t i = 0; i < RSPQ_PROFILE_SLOT_COUNT; i++) {
        ProfileSlot *slot = &slots[i];
        if (slot->name == NULL) continue;
        debugf("  %-20s %5lu %7ld\n", slot->name, slot->calls, slot->timeUs);
    }
    debugf("  %-20s %5s %7ld\n", "Total Wait", "", timeTotalWait);
    debugf("  %-20s %5s %7ld\n", "Total Busy", "", timeTotalBusy);
    debugf("  %-20s %5s %7llu\n", "Total Time", "", totalTicks / profile_data.frame_count);
}

#endif // PROFILE_PRINT_H
#endif // RSPQ_PROFILE
