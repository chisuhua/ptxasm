	.text
	.global _Z9vectorAddIaaaEvPKT_PKT0_PT1_i
	.type _Z9vectorAddIaaaEvPKT_PKT0_PT1_i,@function
_Z9vectorAddIaaaEvPKT_PKT0_PT1_i:
bb_00:
func_begin6:
	ld.param.u64	%vd0, [%s0 + _Z9vectorAddIaaaEvPKT_PKT0_PT1_i_param_0]
	ld.param.u64	%vd2, [%s0 + _Z9vectorAddIaaaEvPKT_PKT0_PT1_i_param_1]
	ld.param.u64	%vd4, [%s0 + _Z9vectorAddIaaaEvPKT_PKT0_PT1_i_param_2]
	ld.param.u32	%v6, [%s0 + _Z9vectorAddIaaaEvPKT_PKT0_PT1_i_param_3]
func_exec_begin6:
tmp12:
	mov.u32	%v7, %ntid.x
	mov.u32	%v8, %ctaid.x
	mul.s32	%v9, %v7, %v8
	mov.u32	%v10, %tid.x
	add.s32	%v11, %v9, %v10
tmp13:
	cmp_tcc.lt.s32	%p0, %v11, %v6
	not_tcc..pred	%p1, %p0
	s_branch_tccnz	%p1, BB6_2
	s_branch BB6_1
BB6_1:
tmp14:
	cvt.s64.s32	%vd6, %v11
	add.s64	%vd8, %vd0, %vd6
	ld.global.u8	%rs1, [%vd8]
	cvt.u32.u16	%v12, %rs1
	cvt.s32.s8	%v13, %v12
	cvt.s64.s32	%vd10, %v11
	add.s64	%vd12, %vd2, %vd10
	ld.global.u8	%rs2, [%vd12]
	cvt.u32.u16	%v14, %rs2
	cvt.s32.s8	%v15, %v14
	add.s32	%v16, %v13, %v15
	cvt.u16.u32	%rs3, %v16
	cvt.s64.s32	%vd14, %v11
	add.s64	%vd16, %vd4, %vd14
	st.global.u8	[%vd16], %rs3
tmp15:
BB6_2:
	t_exit
tmp16:
func_end6:
-
opu.kernels:
 - .name: _Z9vectorAddIaaaEvPKT_PKT0_PT1_i
   .args:
     - .address_space: global .name: _Z9vectorAddIaaaEvPKT_PKT0_PT1_i_param_0 .offset: 0 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: _Z9vectorAddIaaaEvPKT_PKT0_PT1_i_param_1 .offset: 0 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: _Z9vectorAddIaaaEvPKT_PKT0_PT1_i_param_2 .offset: 0 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: _Z9vectorAddIaaaEvPKT_PKT0_PT1_i_param_3 .offset: 0 .size: 8 .value_kind: global_buffer
   .shared_memsize: 0
   .private_memsize: 0
   .cmem_size: 0
   .bar_used: 0
   .local_framesize: 0
   .kernel_ctrl: 73
   .kernel_mode: 0
opu.version:
 - 2
 - 0
...
