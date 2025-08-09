	.text
	.global _Z9vectorAddPfS_S_i
	.type _Z9vectorAddPfS_S_i,@function
_Z9vectorAddPfS_S_i:
bb_00:
	ld.param.u64	%vd0, [%s0 + _Z9vectorAddPfS_S_i_param_0]
	ld.param.u64	%vd2, [%s0 + _Z9vectorAddPfS_S_i_param_1]
	ld.param.u64	%vd4, [%s0 + _Z9vectorAddPfS_S_i_param_2]
	ld.param.u32	%v6, [%s0 + _Z9vectorAddPfS_S_i_param_3]
	ld.global.f32	%v7, [%vd0]
	ld.global.f32	%v8, [%vd2]
	add.f32	%v9, %v7, %v8
	st.global.f32	[%vd4], %v9
	t_exit
-
opu.kernels:
 - .name: _Z9vectorAddPfS_S_i
   .args:
     - .address_space: global .name: _Z9vectorAddPfS_S_i_param_0 .offset: 0 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: _Z9vectorAddPfS_S_i_param_1 .offset: 0 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: _Z9vectorAddPfS_S_i_param_2 .offset: 0 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: _Z9vectorAddPfS_S_i_param_3 .offset: 0 .size: 8 .value_kind: global_buffer
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
