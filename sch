import os
import fitz  # PyMuPDF
import spacy
import pytesseract
from PIL import Image
import cv2
import re
from spacy.matcher import Matcher

# Initialize spaCy
nlp = spacy.load("en_core_web_sm")

def preprocess_image(image_path):
    """Enhance image quality for OCR"""
    img = cv2.imread(image_path)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    processed = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY | cv2.THRESH_OTSU)[1]
    return processed

def extract_drug_names(text):
    """Extract drug names using spaCy NER and pattern matching"""
    doc = nlp(text)
    drugs = []
    
    # Pattern for drug-like words (case-sensitive)
    pattern = [{"POS": "PROPN", "OP": "+"}, {"TEXT": {"REGEX": "[A-Z][a-z]*"}}]
    matcher = Matcher(nlp.vocab)
    matcher.add("DRUG_PATTERN", [pattern])
    
    # Extract NER chemicals
    for ent in doc.ents:
        if ent.label_ == "CHEMICAL":
            drugs.append(ent.text)
    
    # Extract pattern matches
    matches = matcher(doc)
    for match_id, start, end in matches:
        span = doc[start:end]
        drugs.append(span.text)
    
    return list(set(drugs))

def process_pdf(pdf_path):
    """Main processing function"""
    # Create output directory
    protocol_name = os.path.splitext(os.path.basename(pdf_path))[0]
    output_dir = f"Schema_Images_{protocol_name}"
    os.makedirs(output_dir, exist_ok=True)
    
    doc = fitz.open(pdf_path)
    in_schema_section = False
    figure_count = 0
    image_drugs = []
    text_drugs = []
    
    # Extract study design text first
    study_design_text = ""
    for page in doc:
        text = page.get_text()
        if "Study Design" in text:
            study_design_text += text + "\n"
    
    # Process pages for Schema section
    for page_num, page in enumerate(doc):
        text = page.get_text()
        
        # Detect section headers
        if "Schema" in text:
            in_schema_section = True
        elif in_schema_section and "\\x0c" in text:  # Form feed character
            in_schema_section = False
            
        if in_schema_section:
            # Extract images
            img_list = page.get_images()
            for img_index, img in enumerate(img_list):
                figure_count += 1
                xref = img[0]
                base_image = doc.extract_image(xref)
                image_bytes = base_image["image"]
                
                # Save image
                img_path = os.path.join(output_dir, f"Figure {figure_count}.png")
                with open(img_path, "wb") as img_file:
                    img_file.write(image_bytes)
                
                # OCR processing
                processed_img = preprocess_image(img_path)
                ocr_text = pytesseract.image_to_string(processed_img)
                image_drugs.extend(extract_drug_names(ocr_text))
    
    # Extract drugs from study design
    text_drugs = extract_drug_names(study_design_text)
    
    # Cross-validate drug names
    matched = set(image_drugs) & set(text_drugs)
    mismatched = set(image_drugs) ^ set(text_drugs)
    
    print(f"Protocol Name: {protocol_name}")
    print(f"Matched Drug Names: {matched}")
    print(f"Potential Mismatches: {mismatched}")
    
    # Save results to file
    with open(os.path.join(output_dir, "drug_validation.txt"), "w") as f:
        f.write(f"Matched Drug Names: {', '.join(matched)}\n")
        f.write(f"Potential Mismatches: {', '.join(mismatched)}\n")
    
    return output_dir

# Usage
pdf_path = "your_protocol.pdf"
output_directory = process_pdf(pdf_path)
