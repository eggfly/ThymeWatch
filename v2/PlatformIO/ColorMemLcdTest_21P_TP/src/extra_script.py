import os  
from SCons.Script import DefaultEnvironment  

env = DefaultEnvironment()  
platform = env.PioPlatform()  

# 定义 ULP 源文件和生成的二进制文件路径  
ULP_SOURCES = ['src/ulp_spi_example.c']  
ULP_ELF = os.path.join('$BUILD_DIR', 'ulp_main.elf')  
ULP_BIN = os.path.join('$BUILD_DIR', 'ulp_main.bin')  

def compile_ulp_riscv(source, target, env):  
    # 获取工具链路径  
    compiler_path = env.PioPlatform().get_package_dir("toolchain-riscv-esp")  
    ulp_compile_flags = [  
        "-Os",  
        "-mabi=ilp32",  
        "-march=rv32imc",  
        "-Wno-attributes",  
        "-nostartfiles",  
        "-nodefaultlibs",  
        "-nostdlib",  
        "-fno-common",  
        "-ffunction-sections",  
        "-fdata-sections",  
        "-Wall",  
    ]  
    ulp_ld_script = os.path.join(compiler_path, "esp32s3.ulp.ld")  

    # 编译 ULP 源文件  
    for src in ULP_SOURCES:  
        object_file = os.path.join('$BUILD_DIR', os.path.splitext(os.path.basename(src))[0] + '.o')  
        env.Command(  
            object_file,  
            src,  
            [  
                '$CC -c $SOURCE -o $TARGET -I $PROJECT_SRC_DIR -I $PROJECT_SRC_DIR/ulp ' + ' '.join(ulp_compile_flags)  
            ]  
        )  

    # 链接 ULP ELF  
    env.Command(  
        ULP_ELF,  
        [object_file],  
        [  
            '$CC $SOURCES -o $TARGET -T ' + ulp_ld_script + ' -Wl,--gc-sections'  
        ]  
    )  

    # 生成 ULP 二进制文件  
    env.Command(  
        ULP_BIN,  
        ULP_ELF,  
        '$OBJCOPY -O binary $SOURCE $TARGET'  
    )  

# 添加构建步骤  
env.AddPreAction("buildprog", compile_ulp_riscv)