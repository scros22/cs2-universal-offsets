#include <string>
#include <vector>
#include <cstdint>
#include <bit>
 
namespace FName
{
    constexpr uintptr_t gnames_offset = 0x18C59900; 
 
    inline std::uint32_t decrypt_index(std::uint32_t index)
    {
        if (!index) return 0;
        const std::uint32_t t = 1385616517u * (std::rotl(index - 1u, 5) ^ 0x12B1DB4u);
        return (t == 912760722u) ? 394923365u : (t - 912760722u);
    }
 
    inline std::uint32_t resolve_number(std::uint32_t number)
    {
        if (!number) return 0;
        const std::uint32_t t = 1385616517u * (std::rotl(number - 1u, 5) ^ 0x12B1DB4u);
        return (t == 591892594u) ? 715791493u : (t - 591892594u);
    }
 
    inline std::uintptr_t get_entry_ptr(std::uint32_t decrypted_index)
    {
        const std::uint32_t chunk = decrypted_index >> 16;
        const std::uint16_t entry = (std::uint16_t)decrypted_index;
 
        uintptr_t chunk_ptr = hv::read<uintptr_t>(BaseAddress + gnames_offset + (static_cast<uintptr_t>(chunk) + 3570) * 8);
        if (!chunk_ptr) return 0;
 
        return chunk_ptr + (static_cast<uintptr_t>(entry) * 2);
    }
 
    inline void decode_bytes(char* dst, const uint8_t* src, uint32_t len, uint32_t seed)
    {
        uint32_t a4 = seed;
        for (uint32_t i = 0; i < len; i += 4)
        {
            uint32_t v5 = (i + 4 > len) ? (len - i) : 4;
            uint32_t v9 = a4 % v5;
            for (uint32_t j = 0; j < v5; j++)
            {
                uint32_t index = i + (v9 + j) % v5;
                dst[i + j] = (char)((uint8_t)0xCF * ((uint8_t)(std::rotl(src[index], 4) ^ 0x82) - (uint8_t)(a4 ^ 0x20)));
                
                uint32_t x = 8666 * a4 + 1452504;
                a4 = (x ^ (x >> 12)) + (a4 ^ (a4 >> 21));
            }
        }
        dst[len] = '\0';
    }
 
    std::string get_string(std::uint32_t index)
    {
        std::uint32_t decrypted = decrypt_index(index);
        if (!decrypted) return "None";
 
        uintptr_t entry_ptr = get_entry_ptr(decrypted);
        if (!entry_ptr) return "None";
 
        uint16_t header = hv::read<uint16_t>(entry_ptr);
        uint32_t len = (header >> 6) ^ 0x272;
        bool is_wide = (header & 1) != 0;
        
        if (len == 0 || len > 1024) return "None";
 
        if ((header >> 6) == 626)
        {
            uint32_t encoded_num = hv::read<uint32_t>(entry_ptr + 2);
            uint32_t encoded_next = hv::read<uint32_t>(entry_ptr + 6);
            std::string base_name = get_string(encoded_next);
            uint32_t num = resolve_number(encoded_num);
            return (num > 0) ? base_name + "_" + std::to_string(num - 1) : base_name;
        }
 
        uint32_t buffer_size = len * (is_wide ? 2 : 1);
        std::vector<uint8_t> buffer(buffer_size);
        hv::read_buffer(entry_ptr + 2, buffer.data(), buffer_size);
 
        std::vector<char> decoded(buffer_size + 2, 0);
        uint32_t seed = len + (((8666 * len + 1452504)) ^ ((8666 * len + 1452504) >> 12));
        decode_bytes(decoded.data(), buffer.data(), buffer_size, seed);
 
        if (is_wide)
        {
            std::string result;
            wchar_t* wide_data = (wchar_t*)decoded.data();
            for (uint32_t i = 0; i < len; i++)
            {
                result += (char)wide_data[i];
            }
            return result;
        }
 
        return std::string(decoded.data());
    }
}