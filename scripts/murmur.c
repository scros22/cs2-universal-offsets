#include <stdio.h>
#include <string.h>
#include <stdint.h>

static uint32_t Murmur2(const char* data, size_t len, uint32_t seed) {
    const uint32_t m = 0x5BD1E995u;
    const int r = 24;
    uint32_t h = seed ^ (uint32_t)len;
    while (len >= 4) {
        uint32_t k;
        memcpy(&k, data, 4);
        k *= m; k ^= k >> r; k *= m;
        h *= m; h ^= k;
        data += 4; len -= 4;
    }
    switch (len) {
        case 3: h ^= (uint8_t)data[2] << 16; /* fallthrough */
        case 2: h ^= (uint8_t)data[1] << 8;
        case 1: h ^= (uint8_t)data[0]; h *= m;
    }
    h ^= h >> 13; h *= m; h ^= h >> 15;
    return h;
}

int main(void) {
    const char* names[] = {
        "Player.DamageHeadShot.AttackerFeedback",
        "Player.DamageHeadShotArmor.AttackerFeedback",
        "Player.DamageBody.AttackerFeedback",
        "Player.DamageBodyArmor.AttackerFeedback",
        "Player.DamageBody.Knife.AttackerFeedback",
        "Player.DamageBodyArmor.Knife.AttackerFeedback",
        "Player.DeathHeadShot.AttackerFeedback",
        "Player.DeathHeadShotArmor.AttackerFeedback",
        "Player.DeathBody.AttackerFeedback",
        "Player.DeathBodyArmor.AttackerFeedback",
    };
    uint32_t seeds[] = { 0x31415926, 0x00000000, 0xDEADBEEF, 0x12345678 };
    for (size_t s = 0; s < sizeof(seeds)/sizeof(seeds[0]); ++s) {
        printf("--- seed = 0x%08X ---\n", seeds[s]);
        for (size_t i = 0; i < sizeof(names)/sizeof(names[0]); ++i) {
            printf("%-50s %08X\n", names[i], Murmur2(names[i], strlen(names[i]), seeds[s]));
        }
        printf("\n");
    }
    return 0;
}
