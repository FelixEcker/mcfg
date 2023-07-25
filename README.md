# mcfg
marie config library

## Building
### Library
To build the mcfg static library mariebuild (`mb`) is required. If installed simply run
`mb` which will automatically use the `build.mb` file.

If the build succeeds, a file named `libmcfg.a` will be output in the root-directory of
the repository.

### Testing Executable
Building the testing executable required a build of the library (`libmcfg.a`) and mariebuild (`mb`).
If installed run `mb -i testing_build.mb`.

If the build succeeds, an executaable named `mcfg_test` will be output in the root-directory of
the repository.
