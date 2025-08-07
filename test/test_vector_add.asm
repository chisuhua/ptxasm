	.text
	.global _Z9vectorAddPfS_S_i
	.type _Z9vectorAddPfS_S_i,@function
_Z9vectorAddPfS_S_i:
bb_00:
	ld.u64	%rd1, [[_Z9vectorAddPfS_S_i_param_0]]
	ld.u64	%rd2, [[_Z9vectorAddPfS_S_i_param_1]]
	ld.u64	%rd3, [[_Z9vectorAddPfS_S_i_param_2]]
	ld.u32	%r1, [[_Z9vectorAddPfS_S_i_param_3]]
	ld.f32	%f1, [[%rd1]]
	ld.f32	%f2, [[%rd2]]
	add.f32	%f3, %f1, %f2
	st.f32	[[%rd3]], %f3
-
opu.kernels:
 - .args:
 # Parameters would be listed here
 .shared_memsize: 0
 .private_memsize: 0
 .cmem_size: 0
 .bar_used: 0
 # .shared_symbols would be listed here
 .kernel_ctrl: 0
 .kernel_mode: 0
opu.version:
 - 2
 - 0
...
