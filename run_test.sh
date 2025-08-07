#!/bin/bash

# 运行测试
cd test
../build/ptxasm simple_add.ptx simple_add.asm

if [ $? -ne 0 ]; then
    echo "Translation failed!"
    exit 1
fi

echo "Test completed. Check simple_add.asm for output."
cat simple_add.asm
