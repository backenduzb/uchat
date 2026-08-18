import uuid
from io import BytesIO
from PIL import Image
from django.core.files.base import ContentFile

def optimize_pfp(image_field, max_size=(500, 500), quality=80):
    if not image_field:
        return None

    try:
        img = Image.open(image_field)
    except Exception:
        return image_field

    if img.mode in ("RGBA", "LA") or (img.mode == "P" and "transparency" in img.info):
        img = img.convert("RGBA")
    else:
        img = img.convert("RGB")

    img.thumbnail(max_size, Image.Resampling.LANCZOS)

    output = BytesIO()
    img.save(output, format="WEBP", quality=quality, method=6)
    output.seek(0)

    file_name = f"{uuid.uuid4()}.webp"

    return ContentFile(output.getvalue(), name=file_name)