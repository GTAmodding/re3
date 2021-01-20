import os

from PIL import Image

os.chdir(os.path.dirname(os.path.realpath(__file__)))
original = Image.open("logo.png")
icon_sizes = [(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)]
original.save("logo.ico", sizes=icon_sizes)

# Crop logo + make sure that background is white
width, height = original.size
crop_area = (0, height // 6, width, height - height // 6)
cropped_image = original.crop(crop_area)
cropped_size = (crop_area[2] - crop_area[0], crop_area[3] - crop_area[1])
new_image = Image.new("RGBA", cropped_size, "WHITE")
new_image.paste(cropped_image, (0, 0), cropped_image)
new_image.convert("RGB").save("logo.bmp")
