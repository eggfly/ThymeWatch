
from PIL import Image

def main():
    # 读取源图像
    src_image = Image.open('20150031.png').convert('RGB')
    src_width, src_height = src_image.size

    # 创建目标图像（176x176）并填充黑色
    dst_width, dst_height = 176, 176
    dst_image = Image.new('RGB', (dst_width, dst_height), (0, 0, 0))

    # 计算源图像粘贴位置，使其居中
    paste_x = (dst_width - src_width) // 2
    paste_y = (dst_height - src_height) // 2

    # 将源图像粘贴到目标图像上
    dst_image.paste(src_image, (paste_x, paste_y))

    # 遍历目标图像的每个像素
    output_data = []
    for y in range(dst_height):
        for x in range(dst_width):
            r, g, b = dst_image.getpixel((x, y))

            # 将 RGB888 转换为 RGB565
            r5 = (r * 31) // 255
            g6 = (g * 63) // 255
            b5 = (b * 31) // 255

            rgb565 = (r5 << 11) | (g6 << 5) | b5

            # 提取每个颜色分量的最高有效位（MSB）
            r1 = (r5 >> 4) & 0x1  # 红色 MSB
            g1 = (g6 >> 5) & 0x1  # 绿色 MSB
            b1 = (b5 >> 4) & 0x1  # 蓝色 MSB

            # 根据颜色定义组合颜色值
            pixel_value = (r1 << 3) | (g1 << 2) | (b1 << 1) | 0x0

            # 添加到输出数据列表
            output_data.append(pixel_value)

    # 导出为 C 语言数组
    with open('image_data.c', 'w') as f:
        f.write('uint16_t image_data[] = {\n')
        for i, pixel in enumerate(output_data):
            # 每行输出16个像素
            if i % 16 == 0:
                if i != 0:
                    f.write('\n')
                f.write('    ')
            f.write(f'0x{pixel:02X}, ')
        f.write('\n};\n')

if __name__ == '__main__':
    main()

