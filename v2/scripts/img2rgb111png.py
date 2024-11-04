from PIL import Image

def main():
    # 读取源图像
    src_image = Image.open('0001.png').convert('RGB')
    src_width, src_height = src_image.size

    # 创建目标图像（176x176）并填充黑色
    dst_width, dst_height = 176, 176
    dst_image = Image.new('RGB', (dst_width, dst_height), (0, 0, 0))

    # 计算源图像粘贴位置，使其居中
    paste_x = (dst_width - src_width) // 2
    paste_y = (dst_height - src_height) // 2

    # 将源图像粘贴到目标图像上
    dst_image.paste(src_image, (paste_x, paste_y))

    # 创建一个新的图像，用于保存RGB111 8色的PNG图片
    rgb111_image = Image.new('RGB', (dst_width, dst_height))

    # 定义RGB111映射表
    rgb111_palette = {
        0b000: (0, 0, 0),         # 000 - 黑色
        0b001: (0, 0, 255),       # 001 - 蓝色
        0b010: (0, 255, 0),       # 010 - 绿色
        0b011: (0, 255, 255),     # 011 - 青色
        0b100: (255, 0, 0),       # 100 - 红色
        0b101: (255, 0, 255),     # 101 - 品红
        0b110: (255, 255, 0),     # 110 - 黄色
        0b111: (255, 255, 255),   # 111 - 白色
    }

    # 遍历目标图像的每个像素
    rgb565_data = []
    for y in range(dst_height):
        for x in range(dst_width):
            r, g, b = dst_image.getpixel((x, y))

            # 将 RGB888 转换为 RGB565
            r5 = (r * 31) // 255
            g6 = (g * 63) // 255
            b5 = (b * 31) // 255

            rgb565 = (r5 << 11) | (g6 << 5) | b5

            # 添加到RGB565数据列表
            rgb565_data.append(rgb565)

            # 提取每个颜色分量的最高有效位（MSB）
            r1 = (r5 >> 4) & 0x1  # 红色 MSB
            g1 = (g6 >> 5) & 0x1  # 绿色 MSB
            b1 = (b5 >> 4) & 0x1  # 蓝色 MSB

            # 根据颜色定义组合颜色值 (3位)
            pixel_value = (r1 << 2) | (g1 << 1) | b1

            # 获取对应的RGB111颜色
            rgb111_color = rgb111_palette[pixel_value]

            # 设置到新的图像中
            rgb111_image.putpixel((x, y), rgb111_color)

    # 保存RGB111 8色的PNG图片
    rgb111_image.save('rgb111_image.png')

    # 导出RGB565为 C 语言数组
    with open('image_data.c', 'w') as f:
        f.write('uint16_t image_data[] = {\n')
        for i, pixel in enumerate(rgb565_data):
            # 每行输出16个像素
            if i % 16 == 0:
                if i != 0:
                    f.write('\n')
                f.write('    ')
            f.write(f'0x{pixel:04X}, ')
        f.write('\n};\n')

if __name__ == '__main__':
    main()
