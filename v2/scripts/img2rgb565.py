
from PIL import Image

def main():
    # 打开源图像
    source_image = Image.open('watch_face_1.png')

    # 检查源图像尺寸
    source_width, source_height = source_image.size
    if source_width != 144 or source_height != 168:
        print(f"源图像尺寸为 {source_width}x{source_height}，预期尺寸为 144x168")
        return

    # 创建目标图像
    target_width, target_height = 176, 176
    target_image = Image.new('RGB', (target_width, target_height), (0, 0, 0))

    # 计算居中位置
    x_offset = (target_width - source_width) // 2
    y_offset = (target_height - source_height) // 2

    # 将源图像粘贴到目标图像
    target_image.paste(source_image, (x_offset, y_offset))

    # 获取像素数据
    pixels = target_image.load()

    # 存储输出数据的列表
    output_data = []

    # 遍历像素并进行转换
    for y in range(target_height):
        for x in range(target_width):
            r, g, b = pixels[x, y]

            # 转换为 RGB111
            r1 = (r >> 7) & 0x1
            g1 = (g >> 7) & 0x1
            b1 = (b >> 7) & 0x1
            #if (r1 > 0):
            #    print(f"r1 = {r1}, g1 = {g1}, b1 = {b1}")

            # 扩展到 RGB565
            r5 = r1 * 0x1F  # 红色 5 位
            g6 = g1 * 0x3F  # 绿色 6 位
            b5 = b1 * 0x1F  # 蓝色 5 位

            # 组合成 16 位像素值
            pixel_value = (r5 << 11) | (g6 << 5) | b5

            # 添加到输出数据列表
            output_data.append(pixel_value)

    # 导出为 C 语言数组
    with open('image_data.c', 'w') as f:
        f.write('unsigned short image_data[] = {\n')
        for i, pixel in enumerate(output_data):
            # 每行输出12个像素
            if i % 12 == 0:
                if i != 0:
                    f.write('\n')
                f.write('    ')
            f.write(f'0x{pixel:04X}, ')
        f.write('\n};\n')

if __name__ == '__main__':
    main()


