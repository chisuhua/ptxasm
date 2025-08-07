.text
	.global _Z9vectorAddPfS_S_i
	.type _Z9vectorAddPfS_S_i,@function
_Z9vectorAddPfS_S_i:
bb_00:
	ld.param.u64	%vd6, [%s0 + _Z9vectorAddPfS_S_i_param_0]
	ld.param.u64	%vd8, [%s0 + _Z9vectorAddPfS_S_i_param_1]
	ld.param.u64	%vd10, [%s0 + _Z9vectorAddPfS_S_i_param_2]
	ld.param.u32	%v1,  [%s0 + _Z9vectorAddPfS_S_i_param_3]
    ld.global.f32   %v2,  [%vd6]
    ld.global.f32   %v3,  [%vd8]
	add.f32	        %v4, %v2, %v3
	st.f32	        [%vd10], %v3
	t_exit
-
opu.kernels:
 - .name: _Z9vectorAddPfS_S_i
   .args:
     - .address_space: param 
       .name: _Z9vectorAddPfS_S_i_param_0 
       .offset: 0 
       .size: 8 
       .value_kind: global_buffer
     - .address_space: param 
       .name: _Z9vectorAddPfS_S_i_param_1 
       .offset: 0 
       .size: 8 
       .value_kind: global_buffer
     - .address_space: param 
       .name: _Z9vectorAddPfS_S_i_param_2 
       .offset: 0 
       .size: 8 
       .value_kind: global_buffer
     - .address_space: param 
       .name: _Z9vectorAddPfS_S_i_param_3 
       .offset: 0 
       .size: 8 
       .value_kind: global_buffer
   .shared_memsize: 0
   .private_memsize: 0
   .const_memsize: 0
   .bar_used: 0
   .local_framesize: 0
   .kernel_ctrl: 0
   .kernel_mode: 0
opu.version:
 - 2
 - 0
...
