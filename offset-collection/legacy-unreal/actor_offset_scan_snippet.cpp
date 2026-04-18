```cpp
std::uintptr_t get_actor_offset( u64 presistant_level ) {
    uint8_t chunk[ 0x2e0 /* ulevel size */ ]{};
    uintptr_t actors_offset;

    if ( driver::km->read( presistant_level , chunk , sizeof( chunk ) ) )
    {
        for ( size_t i = 0; i < sizeof( chunk ) - 3; i++ )
        {
            if ( chunk[ i ] == 0xEA && chunk[ i + 1 ] == 0x00 && chunk[ i + 2 ] == 0x00 && chunk[ i + 3 ] == 0x00 )
            {
                actors_offset = static_cast< int >( i ) - 0x0C;

                if ( actors_offset >= 0 )
                    printf( "Actors : 0x%X\n" , actors_offset );
                else
                    printf( "Actors : invalid\n" );

                break;
            }
        }
    }
    return actors_offset;
}
```

- Credits to Stern for telling me the logic and doing it HXD

```cpp
/* usage */
auto engine = driver::km->read<u64>( driver::km->m_base_addr + 0x1895F6B8 );
auto viewport = driver::read->read<u64>( engine + 0xae0 );
auto world = driver::read->read<u64>( viewport + 0x78 );
offsets::u_level.actors = get_actor_offset( driver::read->read<u64>( world + 0x40 ) );
```