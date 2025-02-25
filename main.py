import layoutparser as lp
import cv2
import pdf2image
import numpy as np
import pytesseract

def pdf_to_image(pdf_path):
    images = pdf2image.convert_from_path(pdf_path)
    return images[0]

model = lp.Detectron2LayoutModel('lp://PubLayNet/faster_rcnn_R_50_FPN_3x/config',
                                 extra_config=["MODEL.ROI_HEADS.SCORE_THRESH_TEST", 0.5],
                                 label_map={0: "Text", 1: "Title", 2: "List", 3:"Table", 4:"Figure"})

def extract_title(pdf_path):
    # Convert PDF to image
    image = pdf_to_image(pdf_path)
    image_np = np.array(image)

    # Detect layout
    layout = model.detect(image_np)

    # Find the first "Title" block
    title_block = next((block for block in layout if block.type == "Title"), None)

    if title_block:
        # Extract the region of the image corresponding to the title
        x1, y1, x2, y2 = title_block.block.coordinates
        title_image = image_np[int(y1):int(y2), int(x1):int(x2)]

        # Convert the image to grayscale for better OCR results
        gray_title = cv2.cvtColor(title_image, cv2.COLOR_RGB2GRAY)

        # Use pytesseract to extract text from the title image
        title_text = pytesseract.image_to_string(gray_title)

        return title_text.strip()
    else:
        return "No title found in the document."

# Main execution
if __name__ == "__main__":
    pdf_path = "data/input_pdfs/example.pdf"
    title = extract_title(pdf_path)
    print(f"Extracted title: {title}")
