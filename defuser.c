#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t lo = (uint32_t)challenge;
    uint32_t hi = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);

    uint32_t s0 = (hi ^ 0x9471U) + 8U * lo;
    uint32_t s1 = (hi + 4U * mid) ^ 0x786AU;
    uint32_t s2 = (lo + 0x2022U) ^ s1;

    values[0] = s2 ^ s0;
    count = 1;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    values[0] = ((uint32_t)(challenge >> 6) & 0xFU) ^ 0x1FU;

    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t shift = 10U + 5U * (i - 1U);
        uint32_t part =
            ((uint32_t)(challenge >> shift) & 0x3FU) + 0xBU;
        values[i] = (5U * values[i - 1] - i) ^ part;
    }

    count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t selector = (uint32_t)challenge & 7U;
    uint32_t mixed =
        (uint32_t)(challenge >> 32) ^
        (uint32_t)(challenge >> 3);
    uint32_t result = 0;

    switch (selector) {
    case 0:
        result = (mixed ^ 0x4F1FU) + (mixed >> 2U);
        break;
    case 1:
        result = 5U * mixed + 0x461E5U;
        break;
    case 2:
        result = (2U * mixed) ^ 0x2429U;
        break;
    case 3:
        result = 5U * mixed + 0x39E0U;
        break;
    case 4:
        result = 3U * mixed + 0x9ED6U;
        break;
    case 5:
        result = 5U * mixed + 0x31420U;
        break;
    case 6:
        result = (mixed ^ 0x4458U) + (mixed >> 2U);
        break;
    case 7:
        result = (4U * mixed) ^ 0x22AU;
        break;
}

    values[0] = selector;
    values[1] = result;
    count = 2;

    return write_answer(out, cap, values, count);
}

static uint32_t solve_p4_walk(
    uint32_t target,
    uint32_t low,
    uint32_t high,
    uint32_t salt
) {
    uint32_t mid = low + ((high - low) >> 1U);
    uint32_t mixed = mid + salt;

    if (target == mid) {
        return (mid + 0x8FU) ^ salt;
    }

    if (target < mid) {
        uint32_t child = solve_p4_walk(target, low, mid - 1U, salt);
        return (5U * child) ^ mixed ^ 0x9U;
    }

    uint32_t child = solve_p4_walk(target, mid + 1U, high, salt);
    return (5U * child) ^ mixed ^ 0x16U;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t target = (uint32_t)challenge & 0x7FU;
    uint32_t salt =
        (uint32_t)(challenge >> 7U) ^
        (uint32_t)(challenge >> 32U);

    values[0] = solve_p4_walk(target, 0U, 127U, salt);
    count = 1U;

    return write_answer(out, cap, values, count);
}

static const uint32_t p5_values[12] = {
    0x015AU, 0x0716U, 0x0B66U, 0x0925U,
    0x060FU, 0x085FU, 0x0252U, 0x050AU,
    0x0A7DU, 0x0335U, 0x0477U, 0x0020U
};

static const uint32_t p5_addends[12] = {
    0x4B3EU, 0xF9B6U, 0x096BU, 0x6F29U,
    0xFD2CU, 0x0DD3U, 0xBF9EU, 0x5933U,
    0xC7D4U, 0xF070U, 0xB942U, 0x52C6U
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t start = (uint32_t)challenge & 7U;
    uint32_t extra_steps = (uint32_t)(challenge >> 3U) & 3U;

    uint32_t acc =
        (uint32_t)(challenge >> 6U) ^
        (uint32_t)(challenge >> 32U) ^
        0x5C10U;

    uint32_t last_value = 0U;

    for (uint32_t i = 0U; i <= extra_steps; ++i) {
        uint32_t pos = start + i;
        acc = 5U * (acc + p5_addends[pos]);
        acc = acc ^ p5_values[pos];
        last_value = p5_values[pos];
    }

    values[0] = acc;
    values[1] = last_value;
    count = 2U;

    return write_answer(out, cap, values, count);
}

static uint32_t apply_p6(uint32_t index, uint32_t x) {
    switch (index) {
        case 0:
            return (32U * x + 0x6280U) ^ (3U * x);

        case 1:
        case 3:
        case 5:
            return ((x << 3U) | (x >> 29U)) ^ 0x2EB1U;

        case 2:
            return (x ^ 0xF197U) + 8U * x;

        case 4:
            return (x >> 3U) | (x << 29U);

        case 6:
            return (x + 0xFBE9U) ^ (x >> 2U);

        case 7:
            return ((x >> 2U) ^ x) + 0xF7D4U;

        default:
            return 0U;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t x =
        (uint32_t)(challenge >> 6U) ^
        (uint32_t)(challenge >> 32U);

    uint32_t index1 = (uint32_t)challenge & 7U;
    uint32_t index2 = (uint32_t)(challenge >> 3U) & 7U;

    uint32_t first = apply_p6(index1, x);
    uint32_t second = apply_p6(index2, first ^ 0x5BD9U);
    uint32_t answer = second + 0xE828U;

    values[0] = answer;
    count = 1U;

    return write_answer(out, cap, values, count);
}

int solve_secret(
    uint64_t challenge,
    uint64_t context,
    char *out,
    size_t cap
) {
    uint32_t challenge_fold =
        (uint32_t)challenge ^
        (uint32_t)(challenge >> 32);

    uint32_t context_fold =
        (uint32_t)context ^
        (uint32_t)(context >> 32);

    uint32_t selector = (uint32_t)context & 3U;
    uint32_t context_mid = (uint32_t)(context >> 16);

    uint32_t mix_input = challenge_fold ^ context_fold;

    uint32_t rotated = (mix_input << 5U) | (mix_input >> 27U);
    uint32_t secret_value = (rotated + 0x3F8FU) ^ 0x3988U;

    uint32_t first = selector;

    uint32_t second =
        secret_value ^
        (selector * 0x3F8FU) ^
        context_mid;

    uint32_t third =
        second ^
        (context_fold
         + selector
         + 0x3988U
         + (challenge_fold >> 5U));

    int written = snprintf(
    out,
    cap,
    "00000000000000000000000000000000OPEN%"
    PRIu32 " %" PRIu32 " %" PRIu32,
    first,
    second,
    third
    );

    return written < 0 || (size_t)written >= cap;
}