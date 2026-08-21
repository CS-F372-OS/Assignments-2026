# lab-demo


# Get core dumps 
- `ulimit -c unlimited`
- `echo "core" | sudo tee /proc/sys/kernel/core_pattern`

# View core dump
- `gdb trial core.<pid>`


# GDB Commands
### GDB Session for Our Echo Bug

```
(gdb) debugpintos
(gdb) b main
Breakpoint 1 at 0x804812c: file echo.c, line 35.
(gdb) c
Continuing.

Breakpoint 1, main (argc=3, argv=0xbffff694) at echo.c:35
35          printf("<1> Starting main, argc=%d\n", argc);

(gdb) p argc
$1 = 3

(gdb) p argv[1]
$2 = 0xbffff7d4 "this_is_a_very_long_argument_that_will_overflow_our_buffer"

(gdb) p strlen(argv[1])
$3 = 58

(gdb) b unsafe_copy
Breakpoint 2 at 0x8048100: file echo.c, line 28.

(gdb) c
Continuing.

Breakpoint 2, unsafe_copy (dest=0xbffff678, src=0xbffff7d4) at echo.c:28
28          strcpy(dest, src);

(gdb) p dest
$4 = 0xbffff678 ""

(gdb) p src
$5 = 0xbffff7d4 "this_is_a_very_long_argument_that_will_overflow_our_buffer"

(gdb) x/10wx dest
0xbffff678:     0x00000000      0x00000000      0x08048a96
0xbffff681:     0x08048ac8      0xbffff694      0x00000003

(gdb) s
Program received signal SIGSEGV, Segmentation fault.
0x40001234 in strcpy ()
```

### Advanced GDB Debugging

#### Memory Inspection
```
(gdb) x/20wx 0xbffff678    # Examine memory around buffer
(gdb) x/20c 0xbffff678     # View as characters
(gdb) disas unsafe_copy    # Disassemble function
```