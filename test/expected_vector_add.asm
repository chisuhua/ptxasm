    .text
    .global _Z9vectorAddPfS_S_i
    .type _Z9vectorAddPfS_S_i,@function
_Z9vectorAddPfS_S_i:
bb_00:
    // mov.u32 %r2, %tid.x;
    mov.u32	%r2, %tid.x
    // mov.u32 %r3, %ntid.x;
    mov.u32	%r3, %ntid.x
    // mov.u32 %r4, %ctaid.x;
    mov.u32	%r4, %ctaid.x
    // mul.lo.u32 %r5, %r3, %r4;
    mul.lo.u32	%r5, %r3, %r4
    // add.u32 %r6, %r5, %r2;
    add.u32	%r6, %r5, %r2
    // setp.ge.u32 %p1, %r6, %r1;
    setp.ge.u32	%p1, %r6, %r1
    // @%p1 bra BB0_3;
    t_cbranch_tccnz	%p1, bb_03
    // mul.wide.u32 %rd4, %r6, 4;
    // Assuming decomposition or coasm handles .wide
    mul.u32	%rd0, %r6, 4
    mov.u64	%rd4, %rd0
    // ld.global.f32 %f1, [%rd1 + %rd4];
    // %rd1 maps to symbolic register %_Z9vectorAddPfS_S_i_param_0
    ld.f32	%f1, [%_Z9vectorAddPfS_S_i_param_0 + %rd4]
    // ld.global.f32 %f2, [%rd2 + %rd4];
    // %rd2 maps to symbolic register %_Z9vectorAddPfS_S_i_param_1
    ld.f32	%f2, [%_Z9vectorAddPfS_S_i_param_1 + %rd4]
    // add.f32 %f3, %f1, %f2;
    add.f32	%f3, %f1, %f2
    // st.global.f32 [%rd3 + %rd4], %f3;
    // %rd3 maps to symbolic register %_Z9vectorAddPfS_S_i_param_2
    st.f32	[%_Z9vectorAddPfS_S_i_param_2 + %rd4], %f3
    // BB0_3:
    bb_03:
    // ret;
    t_exit
-
opu.kernels:
 - .name: _Z9vectorAddPfS_S_i
   .args:
     - .address_space: global
       .name: _Z9vectorAddPfS_S_i_param_0
       .offset: 0
       .size: 8
       .value_kind: global_buffer
     - .address_space: global
       .name: _Z9vectorAddPfS_S_i_param_1
       .offset: 8
       .size: 8
       .value_kind: global_buffer
     - .address_space: global
       .name: _Z9vectorAddPfS_S_i_param_2
       .offset: 16
       .size: 8
       .value_kind: global_buffer
     - .address_space: global
       .name: _Z9vectorAddPfS_S_i_param_3
       .offset: 24
       .size: 4
       .value_kind: global_buffer
   .shared_memsize: 0
   .private_memsize: 0
   .cmem_size: 0
   .bar_used: 0
   .local_framesize: 0
   .kernel_ctrl: 7
   .kernel_mode: 0
opu.version:
 - 2
 - 0
...
