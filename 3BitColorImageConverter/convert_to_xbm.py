from PIL import Image, ImageOps


def convert_to_1bit(image_path, xbm_path, inverted=False):
    """
    Convert a PNG image to 1-bit XBM format with optional inversion.

    Args:
    image_path (str): The path to the PNG image file.
    xbm_path (str): The path to save the converted XBM file.
    inverted (bool): Whether to invert the colors of the output image (default False).
    """
    with Image.open(image_path) as img:
        # Convert the image to 1-bit format
        img = img.convert("1")

        # Invert the image if the `inverted` argument is True
        if inverted:
            img = ImageOps.invert(img)

        # Save the image in XBM format
        img.save(xbm_path, format="XBM")


# Example usage:
input_file = 'icons/ok/alarm_20x20.png'

convert_to_1bit(input_file, input_file + ".xbm.c")
convert_to_1bit(input_file, input_file + '_inverted.xbm.c', inverted=True)
