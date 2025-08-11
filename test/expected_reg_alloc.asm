    .text
    .global test_reg_alloc
    .type test_reg_alloc,@function
test_reg_alloc:
bb_00:
    ld.param.u64    %vd0, [%s0 + param_a]; // %rd1 mapped to %vd0 (%v0, %v1)
    ld.param.u64    %vd2, [%s0 + param_b]; // %rd2 mapped to %vd2 (%v2, %v3)
    ld.param.u32    %v4, [%s0 + param_c];  // %r1 mapped to %v4

    mov.u32     %v5, %v4;        // %r2 -> %v5
    mov.f32     %v6, %v5;        // %f1 -> %v6
    add.u32     %v7, %v4, %v5;   // %r3 -> %v7
    add.f32     %v8, %v6, %v7;   // %f2 -> %v8

    mov.u64     %vd10, %vd0;      // %rd3 -> %vd4 (%v4, %v5) - Wait, %v4 is taken by %r1!
                                 // Allocator needs to find next free pair: %vd6 (%v6, %v7)? No, taken.
                                 // Next pair: %vd8 (%v8, %v9)? %v8 taken by %f2.
                                 // Next pair: %vd10 (%v10, %v11)? Should be free.
                                 // Corrected expectation: %rd3 -> %vd10 (%v10, %v11)

    add.u64     %vd12, %vd0, %vd10; // %rd4 -> %vd12 (%v12, %v13)

    mov.u32     %v14, %v12;       // %r4 -> %v14 (Conceptual: use low 32 bits of %vd12)
    mov.f32     %v15, %v14;        // %f3 -> %v15

    t_exit
-
opu.kernels:
 - .name: test_reg_alloc
   .args:
     - .address_space: global .name: param_a .offset: 0 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: param_b .offset: 8 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: param_c .offset: 16 .size: 4 .value_kind: global_buffer
   .shared_memsize: 0
   .private_memsize: 0
   .cmem_size: 0
   .bar_used: 0
   .local_framesize: 0
   .kernel_ctrl: 0
   .kernel_mode: 0
opu.version:
 - 2
 - 0
...
