def process_pdf(pdf_path):
    """Main processing function with detailed debugging"""
    # Setup protocol name and folders
    protocol_name = os.path.splitext(os.path.basename(pdf_path))[0]
    output_dir = f"{protocol_name}-schema"
    os.makedirs(output_dir, exist_ok=True)
    debug_print("created output directory", output_dir)

    doc = fitz.open(pdf_path)
    schema_text = []
    study_design_text = []
    image_drugs = []
    figure_count = 0

    # ================== SCHEMA SECTION PROCESSING ==================
    debug_print("processing schema section", "Starting PDF scan")
    in_schema = False
    current_heading = None

    for page_num, page in enumerate(doc):
        debug_print(f"processing page {page_num+1}", "")
        blocks = page.get_text("dict")["blocks"]
        
        for block in blocks:
            if "lines" in block:
                # Check for headings (assuming headings are in larger font)
                for span in block["lines"][0]["spans"]:
                    text = span["text"].strip()
                    font_size = span["size"]
                    
                    # Detect headings based on common heading font sizes
                    if font_size > 11 and text.isupper():
                        debug_print("heading detected", f"'{text}' (size: {font_size})")
                        
                        if text == "SCHEMA":
                            in_schema = True
                            current_heading = "SCHEMA"
                            debug_print("entered schema section", "")
                        elif in_schema and current_heading == "SCHEMA":
                            debug_print("exiting schema section", f"New heading: {text}")
                            in_schema = False
                        elif text == "STUDY DESIGN":
                            current_heading = "STUDY DESIGN"
                            debug_print("entered study design section", "")

                    # Collect text based on current section
                    if in_schema and current_heading == "SCHEMA":
                        schema_text.append(text)
                    elif current_heading == "STUDY DESIGN":
                        study_design_text.append(text)

        # Extract images only in schema section
        if in_schema and current_heading == "SCHEMA":
            img_list = page.get_images()
            for img_index, img in enumerate(img_list):
                figure_count += 1
                xref = img[0]
                base_image = doc.extract_image(xref)
                image_bytes = base_image["image"]
                
                # Save image
                img_name = f"Figure {figure_count}.png"
                img_path = os.path.join(output_dir, img_name)
                with open(img_path, "wb") as img_file:
                    img_file.write(image_bytes)
                debug_print("image saved", img_path)
                
                # OCR processing
                ocr_text = pytesseract.image_to_string(Image.open(img_path))
                debug_print(f"ocr text from {img_name}", ocr_text)
                image_drugs.extend(extract_drug_names(ocr_text))

    # ================== STUDY DESIGN PROCESSING ==================
    debug_print("processing study design section", "")
    study_design_full = " ".join(study_design_text)
    text_drugs = extract_drug_names(study_design_full))

    # ================== VALIDATION ==================
    schema_full_text = " ".join(schema_text)
    schema_drugs = extract_drug_names(schema_full_text)
    image_drugs = list(set(image_drugs))
    
    debug_print("schema drugs", schema_drugs)
    debug_print("image drugs", image_drugs)
    debug_print("study design drugs", text_drugs)

    # Cross-checking
    all_image_drugs = list(set(schema_drugs + image_drugs))
    matches = set(all_image_drugs) & set(text_drugs)
    mismatches = set(all_image_drugs).symmetric_difference(text_drugs)

    # Save debug files
    with open(os.path.join(output_dir, "schema_text.txt"), "w") as f:
        f.write(schema_full_text)
    with open(os.path.join(output_dir, "study_design.txt"), "w") as f:
        f.write(study_design_full)

    debug_print("final matches", matches)
    debug_print("potential mismatches", mismatches)

    return {
        "matches": list(matches),
        "mismatches": list(mismatches)
    }
