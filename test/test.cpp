#include <clobber/clobber.h>

inline int return_something(int somevalue) {
    int acc = 0;
    for (int i = 0; i < somevalue; ++i)
        acc += i*i;
    return acc;
}

int main() {
    clobber::DoNotOptimize(return_something(4));

    clobber::ClobberMemory();

    clobber::internal::UseCharPointer((char*)return_something(4));
    return 0;
}
