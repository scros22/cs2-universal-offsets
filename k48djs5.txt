# **Fortnite Offsets** -- `++Fortnite+Release-40.10-CL-52157884`
Dumped by **SnipeHype200(frenchbulldogs)**

## **Core Offsets**
```cpp
// UWorld
constexpr uintptr_t UWorld           = 0x1895DE60;
constexpr uintptr_t GNames           = 0x18814B40;
constexpr uintptr_t GObjects         = 0x188EF210;
constexpr uintptr_t GObjectsCount    = 0x188EF220;
```

## **Pattern Offsets**
```cpp
constexpr uintptr_t BONE_ARRAY             = 0x628;
constexpr uintptr_t CAMERA_LOCATION         = 0x170;
constexpr uintptr_t CAMERA_ROTATION         = 0x180;
constexpr uintptr_t UVIEWSTATE_FPV_RADIANS  = 0x740;
constexpr uintptr_t UVIEWSTATE_MPROJECTION  = 0x940;
constexpr uintptr_t COMPONENT_TO_WORLD      = 0x1E0;
constexpr uintptr_t DELTA_TIME_SECONDS       = 0x88C;
constexpr uintptr_t TIME_SECONDS             = 0x868;
constexpr uintptr_t MESH_LAST_RENDER_TIME   = 0x330;
constexpr uintptr_t PLAYER_NAME             = 0xA08;
```

## **UWorld Decryption**
```cpp
inline uint64_t Decrypt_Uworld(uint64_t Pointer)
{
    Pointer ^= 0xCF76574CULL;
    Pointer = std::rotl(Pointer, 48);
    Pointer = ~Pointer;
    return Pointer;
}
```

## **GEngine UWorld Chain**
```cpp
constexpr uintptr_t GEngine          = 0x1895F6B8;
constexpr uintptr_t VIEWPORT_CLIENT   = 0xAE0; // GEngine->ViewportClient
constexpr uintptr_t VIEWPORT_UWORLD   = 0x78; // ViewportClient->World

// Usage: Read(Base + GEngine) -> +VIEWPORT_CLIENT -> +VIEWPORT_UWORLD = UWorld
```

## **Visible Check**
```cpp
inline bool is_visible(uintptr_t mesh)
{
    auto delta_time_seconds = driver.Read<float>(Addresses::UWorld + 0x88C);
    auto time_seconds = driver.Read<double>(Addresses::UWorld + 0x868);

    auto last_render_time = driver.Read<float>(mesh + 0x330);
    auto time_since = time_seconds - last_render_time;
    auto render_time_threshold = fmaxf(0.06f, delta_time_seconds + 0.000099999997f);

    return time_since <= render_time_threshold;
}
```