# test286.asm

**test286.asm** is an 80286 CPU tester, written for the [NASM](https://www.nasm.us/)
assembler. It runs as a BIOS replacement and does not depend on any OS or 
external library. It is a 16-bit port of 
[test386.asm](https://github.com/barotto/test386.asm) by Marco Bortolin.

The ROM communicates through a diagnostic POST I/O port and a custom ASCII
output port. You'll need to configure the addresses of these ports for the 
system you're testing. 

No video, parallel/serial controller, PIC, PIT, DMA controller or FPU needs to
be initialized or attached.

## Project status

**test286.asm** does not test every aspect of the 80286. It
covers real and protected mode, privilege transitions, 286 task switching,
and what byte/word arithmetic operations could be retained from test386. The 
full default test suite has completed on a physical Harris 80C286.

See [How to use](#how-to-use) for the tests and their diagnostic codes.
The default build runs self-checking tests followed by **44 BCD observations**
and **14,561 arithmetic rows** from 161 operation entries. Instructions without
dedicated checks are also used throughout the ROM; errors in those instructions
can cause crashes or loops.

The tests target the 80286 instruction set, using 286 descriptors, gates and
44-byte task-state segments. They omit 386-only functionality such as 32-bit
operands and addressing, paging, virtual-8086 mode, `FS`/`GS`, SETcc, bit 
scan/test instructions and SHLD/SHRD. After POST `08`, **test286.asm** remains
in protected mode until it halts.

### Realistic expectations

Passing a finite test suite cannot establish that a CPU implementation is
correct for every input and machine state. This port covers instruction
families, but not every descriptor-fault precedence rule, 
privilege/stack-boundary combination, or task-gate/double-fault path.

POST `FF` confirms that the self-checking tests passed and execution completed.
The arithmetic observation stream requires comparison with a reference.
The supplied reference records one physical CPU's behavior (the Harris 80C286); 
matching it does not establish correctness for every 286 implementation or
stepping.

### Other projects

- [SingleStepTests/80286](https://github.com/SingleStepTests/80286) - a set of
  emulator CPU tests for the Intel 80286 by Daniel Balsom (gloriouscow).
- [s a n d s i f t e r](https://github.com/xoreaxeaxeax/sandsifter) - an x86
  processor fuzzer by Christopher Domas.
- [test386.asm](https://github.com/barotto/test386.asm) - a test ROM for the
  386, by Marco Bortolin, from which this project is derived.

## How to assemble

Install [NASM](https://www.nasm.us/) and [CMake 3.20](https://cmake.org/download/)
or newer. These are the only build tools required and should be on PATH.

Direct NASM builds use the defaults in
[src/configuration.inc](src/configuration.inc). CMake supplies the defaults
below on each invocation; override them using `-D` options:

| CMake option            | Default | Purpose                                                                                           |
| ----------------------- | ------- | ------------------------------------------------------------------------------------------------- |
| `POST_PORT`             | `0x190` | Diagnostic POST port                                                                              |
| `OUT_PORT`              | `0x191` | ASCII output port; zero disables text                                                             |
| `TRACE_ARITH`           | `ON`    | Emit the `EE` arithmetic observation stream                                                       |
| `INJECT_FAILURE`        | `OFF`   | Deliberately fail an assertion at POST `02`                                                       |
| `RESET_ON_TRIPLE_FAULT` | `OFF`   | Test return to real mode and general-register preservation when the system resets on triple fault |

From the repository root:

```sh
cmake -P build.cmake
```

The script invokes NASM directly on every run. Pass `-D` options before `-P`;
options are supplied per invocation and are not cached. 

Select an assembler with `-DNASM_EXECUTABLE=/path/to/nasm`.

The default output directory is `output/` beside `build.cmake`; use 
`-DOUTPUT_DIR=...` to specify an alternate output directory. 

Explicit relative output paths are relative to the current working directory.
For example, to run only the self-checking tests while retaining the arithmetic
assertions:

```sh
cmake -DTRACE_ARITH=OFF -DOUTPUT_DIR=output/selfcheck -P build.cmake
```

The direct NASM equivalent for the default ROM, from the same directory, is:

```sh
nasm -I src/ -f bin -Wall -Werror -l test286.lst -o test286.bin src/test286.asm
```

For direct assembly, override configuration values with NASM `-D` defines,
using `0` or `1` for switches.

If the emulator or system asserts RESET when the CPU enters shutdown after a
triple fault, enable the optional POST `07` tests:

```sh
cmake -DRESET_ON_TRIPLE_FAULT=ON -DOUTPUT_DIR=output/reset -P build.cmake
```

The NASM equivalent is `-DRESET_ON_TRIPLE_FAULT=1`. These tests deliberately
triple-fault twice, using complementary patterns in `AX`, `CX`, `DX`, `BX`,
`SP`, `BP`, `SI` and `DI`. Each reset-vector continuation checks all eight
registers before changing one or using the stack, then checks that `MSW.PE`
is clear. The preservation expectation follows the Harris 80C286 RESET
microcode, which has no decoded writes to these registers.

RAM must survive both resets. Reserve physical bytes 00580h-00585h for the
continuation record and clear them before a new, independent run. The ROM
arms this record before the test and clears it after the second continuation.

With this option disabled, no shutdown/reset handling is required.

### Output 

The final product is **test286.bin**, exactly **65,536 bytes**. 

CMake also produces `test286.lst` and `build.json`, checks the ROM size and
far-jump opcode at the reset vector, and records source hashes and build options.
The image begins with a NUL-terminated version string (`test286.asm v1.2.0`);
the release date is generated at assembly time.


## How to install

Install the binary at physical address **0xF0000** and alias it at
**0xFF0000**, the top 64 KiB of the 286's 24-bit address space. The reset-vector
jump aligns with **0xFFFFF0** and transfers control to **F000:0013**, immediately
after the version header.

After this jump, all memory accesses remain within the first 1 MiB.

## How to use

Provide writable RAM below 0xA0000. No existing BIOS or peripheral 
initialization is required. POST and text writes can be recorded by the emulator
or a bus monitor. Keep external interrupt inputs inactive; the tests change `IF`
while checking flags and privilege transitions.

Testing starts immediately after RESET. A successful run prints `PASS` via the
ASCII output port, emits `FF` to the POST port, and then halts. With
`OUT_PORT=0`, text output is disabled, but POST reporting remains enabled.

On an assertion failure, the current POST code identifies the test family and 
the common error routine prints `FAIL` via the ASCII port, if enabled, then
halts. No assertion ID is printed. Some conditional-jump checks halt directly
at POST `01` without printing `FAIL`.

Use the NASM listing **test286.lst** and your emulator's execution log to locate
the failing instruction. Break at `error` or a failing `HLT`, then use the
instruction trace to find the comparison, branch or exception that led there.
Match the code-segment `IP` to the listing's offset column.

An unexpected reset or endless loop can also indicate a failure in stack
handling, CALL/RET, or exception entry.

With `RESET_ON_TRIPLE_FAULT=ON`, the ROM triggers two resets during POST `07`.
It uses a continuation record in RAM to check the results after each reset,
then automatically continues at POST `08` without repeating the earlier tests.

Unexpected exceptions normally enter the common error routine. A wrong error
code or saved `CS:IP` in an expected-fault test also branches there. These failures
retain the current POST code and print the same `FAIL` message; there are no
separate numeric exception diagnostics.

Before starting the ring-3 tests, the ROM writes the byte `20` to
the configured POST port (default **0x190**). This code identifies the ring-3
test phase.

The first ring-3 check runs with `IOPL=3` and, with text output enabled, writes
`.` to the ASCII port. The remaining privilege checks run with `IOPL=0` and
verify faults from CLI, HLT and I/O. A ring-3 assertion failure enters ring 0
through the dedicated `INT 2Fh` gate before printing `FAIL` and halting.
No new POST code is written, so **`20` remains the last value sent to the POST
port** for a failure in this phase. Later tests also enter ring 3 and retain
their own current POST code on failure.

These are the tests in execution order. POST values mostly follow `test386.asm`.
The sequence is not numerical: the ring and task tests (`20` and `22`) run
immediately after the stack tests (`09`). The optional `07` phase briefly
enters protected mode for each RESET test. At `08`, protected mode is entered
for all subsequent tests. BOUND runs once, at `18`.

| POST | Description                                                                                                                      |
| ---- | -------------------------------------------------------------------------------------------------------------------------------- |
| `00` | Real-mode initialization: stack, IVT and data segments                                                                           |
| `01` | Conditional jumps, JCXZ, LOOP wrap and LOOPE/LOOPNE termination                                                                  |
| `02` | Quick word IMUL execution and a MUL/DIV round-trip check; optional deliberate failure                                            |
| `03` | Real-mode segment-register moves through registers and memory; MOV-to-`CS` #UD faults                                            |
| `04` | Byte/word string operations in real mode: both directions, index wrap, zero-count REP and REPE/REPNE early termination           |
| `05` | Direct/indirect near and far CALL/RET and stack-depth checks in real mode                                                        |
| `06` | LDS/LES pointer loads in real mode                                                                                               |
| `1D` | Real-mode CMC, CLC/STC, CLD/STD and CLI/STI; preservation of unrelated flags                                                     |
| `1E` | LGDT/SGDT and LIDT/SIDT round trips with 24-bit bases, 16-bit limits and output-buffer guards                                    |
| `1F` | XLAT for all 256 indices with zero/nonzero `BX` bases; preservation of `AH`, `BX` and flags                                      |
| `23` | Real-mode software INT/IRET: saved `CS:IP` and flags, stack depth, handler execution and restored `SP`/`IF`                      |
| `07` | Optional triple-fault RESETs; preservation of all eight general registers and return to real mode                                |
| `08` | GDT/IDT/LDT setup, protected-mode entry, LLDT/LTR and LMSW register/memory checks that `PE` cannot be cleared                    |
| `09` | Register, segment, immediate and memory PUSH/POP; PUSH `SP`, PUSHA/POPA and PUSHF/POPF                                           |
| `20` | Ring transitions, `IOPL` permissions, privileged-instruction faults, interrupt frames/`IF` and call-gate parameter copying       |
| `22` | INT/task-gate/IRET/JMP task switches; registers, `TR`/`LDTR`, backlink, `NT`/busy, busy-TSS LTR #GP and `TS`/CLTS readback       |
| `0B` | Protected-mode segment-register moves; #UD/#GP/#NP/#SS for invalid selectors, types, privileges and presence                     |
| `0C` | Immediate-byte PUSH sign extension, masking to a byte, and CBW/CWD                                                               |
| `0D` | 16-bit LEA addressing forms and displacements                                                                                    |
| `0F` | Base/index memory addressing, `BP`/`SS` defaults and explicit `DS`/`ES` overrides                                                |
| `10` | Byte/word string operations in protected mode: both directions, index wrap, zero-count REP and REPE/REPNE early termination      |
| `12` | Read-only data; `DS`/`ES` expand-up/down limits `0000h`/`FFFFh`, byte/word boundaries and faulting-write preservation; #GP/#SS   |
| `16` | Direct/indirect near and far CALL/RET and stack-depth checks in protected mode                                                   |
| `17` | ARPL with register/memory operands, changed/unchanged `RPL` and an unchanged read-only destination                               |
| `18` | BOUND within limits and above the upper limit; #BR handler adjusts the bound and IRET retries the instruction                    |
| `19` | Register/memory XCHG, including `SP`                                                                                             |
| `1A` | ENTER allocation and frame copies at nesting levels 0, 1, 4 and 36 (masked to 4)                                                 |
| `1B` | LEAVE stack-frame restoration                                                                                                    |
| `1C` | VERR/VERW and register/memory LAR/LSL at `CPL` 0/3: selector/type/privilege checks, rights, limits, failed-query results         |
| `EE` | 44 BCD observations, followed by 14,561 arithmetic/logical observations from 161 operation entries                               |
| `FF` | Testing completed                                                                                                                |

Test `EE` prints computational results without checking them against expected
values during execution. Reaching POST `FF` confirms that the self-checking
tests passed and execution completed; it does not validate the observation
results. `TRACE_ARITH=OFF` skips both the BCD and arithmetic observation streams
but still runs the preceding self-checking tests and emits `EE` followed by `FF`.

After reaching POST `FF` with the default build, compare the captured text with
**[test286-EE-reference.txt](test286-EE-reference.txt)**. Save the output from
the first BCD row through the final `PASS` as `my-EE-output.txt`, omitting the
earlier `.` from the `IOPL` test and converting CRLF line endings to LF.
From this directory:

```sh
diff my-EE-output.txt test286-EE-reference.txt
```

Under Windows, use a diff tool of your choice. The supplied reference includes
all 44 BCD and 14,561 arithmetic rows and the final `PASS`, with LF line endings;
it excludes the leading `.` from the `IOPL` test. Capture the same scope with
text output enabled. There is no opening banner. The results were recorded
on a Harris 80C286.

Compare the ASCII-port bytes, excluding POST writes and capture-tool messages.
The ROM emits observation rows ending with LF and a final `PASS` ending with
CRLF. Preserve trailing spaces when normalizing the capture for comparison.

Each arithmetic row contains:

- Operation opcode and mnemonic, followed by `B` or `W` for byte or word size.
- `AX`, `DX` and masked `FLAGS` (`PS`) before execution, each as four hexadecimal digits.
- `AX`, `DX` and masked `FLAGS` after execution, in the same order.
- An optional `#DE` marker when the operation raised a divide error.

BCD rows contain the mnemonic, then `AX` and masked `FLAGS` (`PS`) before and after
execution; they have no opcode, size or `DX` field.

Use the `test286.lst` listing and your emulator's execution log to investigate 
any differences. 

The ROM entry point is [src/test286.asm](src/test286.asm).

### Undefined Behaviour

The test386 optional POST `E0` undefined-behavior suite is not implemented in
this port. Its 386-specific assumptions about 32-bit segment pushes, POPAD,
SIB addressing and undefined flags are not adopted as 286 requirements.

The self-checking cases avoid assertions on undefined arithmetic flags. The
`EE` stream retains the test386 operation categories and masks in
[src/tests/arith-logic_d.inc](src/tests/arith-logic_d.inc). Those masks do not
make every recorded bit an architectural guarantee - differences in undefined
flag states are possible between steppings of the same architecture.

Treat such values as observations of the sampled CPU. A reference mismatch
needs investigation before it can be classified as an emulator error.

### Comparing memory dumps

With the same ROM, initial state and external inputs, test286.asm follows a
repeatable test sequence. POST `FF` does not imply that all RAM will be identical
between runs: the ROM does not initialize all RAM, and saved CPU state can differ.

If you have a known working emulator that you trust (preferably open-source),
you can compare its memory dump with yours.

Memory dumps can help locate a failing write or descriptor update. Use the
same ROM, build options, initial memory contents and external inputs, and
capture memory at the same execution point. For a completed run, stop at the
first HLT before any harness recovery changes RAM.

No reference RAM dump is supplied. Compare the regions written by the tests
with a trusted execution, accounting for untouched memory and CPU-dependent
results. This is the memory map (hexadecimal physical addresses):

| Physical range              | Use                                                                       |
| --------------------------- | ------------------------------------------------------------------------- |
| 00000-003FF                 | Real-mode IVT; vectors 0 through 16 are initialized                       |
| 00400-0057F                 | Protected-mode IDT, vectors 00h through 2Fh                               |
| 00580-00585                 | Continuation record when `RESET_ON_TRIPLE_FAULT` is enabled               |
| 00600-007FF                 | GDT allocation                                                            |
| 00A00-00FFF                 | LDT allocation                                                            |
| 05000-0502B                 | Current task's 44-byte 286 TSS                                            |
| 05200-0522B                 | Second task's 44-byte 286 TSS                                             |
| 10000-1FFFF                 | Real-mode and protected-mode stacks; ring stacks use separate `SP` values |
| 20000-9FFFF                 | Test data and stack fixtures; data-segment bases advance between tests    |
| 30000-3FFFF                 | Second task's stack, within the test data area above                      |
| F0000-FFFFF / FF0000-FFFFFF | Identical ROM aliases                                                     |

Use zero-based physical offsets when reporting differences. An emulator memory
breakpoint on the differing location can identify the instruction responsible;
look up its `IP` in `test286.lst` to understand the test's intent.

## Copyright

test286.asm is derived from [test386.asm](https://github.com/barotto/test386.asm),
originally developed for [IBMulator](http://barotto.github.io/IBMulator) as a
derivative of [PCjs](https://www.pcjs.org/). 

See [AUTHORS](AUTHORS).

Distributed under the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version. See [COPYING](COPYING).
