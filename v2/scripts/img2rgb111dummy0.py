
from PIL import Image


def main():
    name = 'calendar'
    # 读取源图像
    src_image = Image.open('%s.png' %name).convert('RGB')
    src_width, src_height = src_image.size

    # 创建目标图像（176x176）并填充黑色
    dst_width, dst_height = 176, 176
    dst_image = Image.new('RGB', (dst_width, dst_height), (0, 0, 0))

    # 计算源图像粘贴位置，使其居中
    paste_x = (dst_width - src_width) // 2
    paste_y = (dst_height - src_height) // 2

    # 将源图像粘贴到目标图像上
    dst_image.paste(src_image, (paste_x, paste_y))

    # 初始化输出数据列表
    output_data = []

    # 遍历目标图像的每个像素
    for y in range(dst_height):
        for x in range(0, dst_width, 2):
            # 处理两个像素
            pixels = []
            for i in range(2):
                if x + i < dst_width:
                    r, g, b = dst_image.getpixel((x + i, y))
                else:
                    r, g, b = 0, 0, 0  # 超出范围的像素，填充黑色

                # 提取每个颜色分量的最高有效位（MSB）
                r_bit = (r >> 7) & 0x1
                g_bit = (g >> 7) & 0x1
                b_bit = (b >> 7) & 0x1

                # 组合RGB111的3位颜色值，最高位为dummy位0
                pixel_value = (r_bit << 3) | (g_bit << 2) | (b_bit << 1)

                pixels.append(pixel_value & 0x0F)  # 确保是4位

            # 将两个像素合并为一个字节
            combined_byte = (pixels[0] << 4) | pixels[1]
            output_data.append(combined_byte)

    # 导出为 C 语言数组
    with open('%s_rgb111.h' %name, 'w') as f:
        f.write('#pragma once\n')
        f.write('#include <stdint.h>\n\n')
        f.write('uint8_t %s_rgb111[] = {\n' %name)
        for i, byte in enumerate(output_data):
            # 每行输出16个字节
            if i % 16 == 0:
                if i != 0:
                    f.write('\n')
                f.write('    ')
            f.write(f'0x{byte:02X}, ')
        f.write('\n};\n')


if __name__ == '__main__':
    main()
