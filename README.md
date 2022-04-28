# Tailslide LScript harness

## Summary

Integrates Tailslide into lscript instead of using the lscript_tree compiler. Supports both CIL and LSO compilation.

## Building

Builds on Linux using [Linden autobuild](https://bitbucket.org/lindenlab/autobuild/src) with
[Firestorm's autobuild variables](https://vcs.firestormviewer.org/fs-build-variables). Clang must be used for the
libFuzzer integration. Not tested elsewhere.

You'll also need to `sudo make install` tailslide itself for now as it doesn't have an autobuild package.

```bash
$ CXX=clang++ autobuild build -A64 -c RelWithDebInfoFS_open
```

## Using

Two main entrypoints are provided:

### lscript_harness

Compiles a provided script to CIL or LSO using either the lscript_tree compiler or tailslide, depending on which
environment variables are present. Scripts compiled using LSO are also executed using `lscript_execute` after compilation:

```bash
USE_TAILSLIDE=1 ./build-linux-x86_64/sharedlibs/bin/lscript_harness indra/lscript/lscript_execute/tests/lsl_conformance.lsl
All tests passed
2022-05-03T19:45:48Z INFO # lscript/lscript_execute/lscript_execute.cpp(4117) lscript_run : 13285 instructions in 0.000804 seconds
2022-05-03T19:45:48Z INFO # lscript/lscript_execute/lscript_execute.cpp(4118) lscript_run : 16523.6K instructions per second
ip: 0x0
sp: 0x3FFF
bp: 0x3FFB
hr: 0x3C3D
hp: 0x3EBF
faults 0
```

```bash
USE_TAILSLIDE=1 COMPILE_CIL=1 ./build-linux-x86_64/sharedlibs/bin/lscript_harness indra/lscript/lscript_execute/tests/lsl_conformance.lsl
file written to /tmp/whatever.cil
```

### lscript_fuzzer

A libFuzzer harness that ensures tailslide produces identical results for any script that lscript_tree is able to compile.
Should be passed a directory of LSL scripts to use as a seed corpus for the fuzzer. Supports fuzzing both LSO and CIL
targets.

```bash
./build-linux-x86_64/sharedlibs/bin/lscript_fuzzer -fork=8 fuzz_corpus/
INFO: Seed: 360814679
INFO: Loaded 1 modules   (49541 inline 8-bit counters): 49541 [0x7f6620, 0x8027a5),
INFO: Loaded 1 PC tables (49541 PCs): 49541 [0x8027a8,0x8c3ff8),
INFO: -fork=8: fuzzing in separate process(s)
INFO: -fork=8: 2754 seed inputs, starting to fuzz in /tmp/libFuzzerTemp.426379.dir
#53: cov: 6962 ft: 37880 corp: 2754 exec/s 17 oom/timeout/crash: 0/0/0 time: 61s job: 1 dft_time: 0
#424: cov: 6962 ft: 37880 corp: 2754 exec/s 123 oom/timeout/crash: 0/0/0 time: 62s job: 2 dft_time: 0
#2465: cov: 6962 ft: 37880 corp: 2754 exec/s 510 oom/timeout/crash: 0/0/0 time: 62s job: 3 dft_time: 0
#3668: cov: 6962 ft: 37940 corp: 2755 exec/s 240 oom/timeout/crash: 0/0/0 time: 63s job: 4 dft_time: 0
#4313: cov: 6962 ft: 37940 corp: 2755 exec/s 107 oom/timeout/crash: 0/0/0 time: 64s job: 5 dft_time: 0
#6321: cov: 6962 ft: 37942 corp: 2756 exec/s 286 oom/timeout/crash: 0/0/0 time: 65s job: 6 dft_time: 0
# ...
```

## License

LGPL v2.1
