# Clobber - Some helper functions for deterministic code execution

Some information will be provided on this page soon.

In the mean time, you can use the library as follows:
 - In a project using CMake, use [CPM] to import the library as a dependency:
```cmake
CPMAddPackage(
    NAME clobber
    GITHUB_REPOSITORY gabrieleara/clobber
    GIT_TAG some.version.number
)
```

 - then link it against your project like so:
```cmake
target_link_libraries(target_name PUBLIC clobber::clobber)
```

<!-- Links -->
[CPM]: https://github.com/cpm-cmake/CPM.cmake
