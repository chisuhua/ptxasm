	.text
	.global _Z10vectorSwapPiS_
	.type _Z10vectorSwapPiS_,@function
_Z10vectorSwapPiS_:
bb_00:
	mov.u32	%r2, %ntid.x
	mov.u32	%r3, %ctaid.x
	mul.s32	%r4, %r2, %r3
	mov.u32	%r5, %tid.x
	add.s32	%r1, %r4, %r5
	setp.lt.u32	%p1, %r1, 32
	not.pred	%p2, %p1
	s_branch BB6_1
	cvt.s64.s32	%rd3, %r1
	shl.b64	%rd4, %rd3, 2
	add.s64	%rd5, %rd1, %rd4
	ld.u32	%r6, [%rd5]
	mov.u32	%r7, _ZZ10vectorSwapPiS_E3Tmp
	cvt.u64.u32	%temp, %r7
	cvt.s64.s32	%rd7, %r1
	shl.b64	%rd8, %rd7, 2
	add.s64	%rd9, %rd6, %rd8
	st.u32	[%rd9], %r6
	setp.ge.u32	%p3, %r1, 32
	not.pred	%p4, %p3
	s_branch BB6_4
	sub.s32	%r8, %r1, 32
	mov.u32	%r9, _ZZ10vectorSwapPiS_E3Tmp
	cvt.u64.u32	%temp, %r9
	cvt.s64.s32	%rd11, %r8
	shl.b64	%rd12, %rd11, 2
	add.s64	%rd13, %rd10, %rd12
	ld.u32	%r10, [%rd13]
	sub.s32	%r11, %r1, 32
	cvt.s64.s32	%rd14, %r11
	shl.b64	%rd15, %rd14, 2
	add.s64	%rd16, %rd2, %rd15
	st.u32	[%rd16], %r10
	t_exit
-
opu.kernels:
 - .name: _Z10vectorSwapPiS_
   .args:
     - .address_space: global .name: _Z10vectorSwapPiS__param_0 .offset: 0 .size: 8 .value_kind: global_buffer
     - .address_space: global .name: _Z10vectorSwapPiS__param_1 .offset: 0 .size: 8 .value_kind: global_buffer
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
