#define ALI2_IMPLEMENTATION
#include "ali2.h"

typedef struct {
    ali_usize data_size;
    ali_usize count;
    void* data;
}AliSlice;

#define ali_slice_is_of_type(slice, Type) ((slice).data_size == sizeof(Type))

#define ali_da_slice(da) ((AliSlice) { .data_size = sizeof((da).items[0]), .count = (da).count, .data = (da).items })
#define ali_sv_slice(da) ((AliSlice) { .data_size = 1, .count = (sv).count, .data = (sv).items })
AliSlice ali_slice_slice(AliSlice slice, ali_usize start, ali_usize end) {
    ali_assert(start < slice.count);
    ali_assert(end <= slice.count);
    ali_assert(start < end);

    return (AliSlice) {
        .data_size = slice.data_size,
        .count = end - start,
        .data = slice.data + (slice.data_size * start),
    };
}

void* ali_slice_get(AliSlice slice, ali_usize index) {
    ali_assert(index < slice.count);
    return slice.data + slice.data_size * index;
}

#define ali_slice_foreach(slice, Type, ptr) for (Type* ptr = (slice).data; ptr < (slice).data + (slice).count * (slice).data_size; ptr++)

#define da_slice ali_da_slice
#define sv_slice ali_sv_slice
#define slice_slice ali_slice_slice
#define slice_get ali_slice_get
#define slice_foreach ali_slice_foreach

int main(void) {
    return 0;
}
