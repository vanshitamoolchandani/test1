def schemaExtarctor(file_path):
        r'(?:\(?\s*(Arm|Cohort)\s+([\dA-Za-z]+)\s*\)?)(?::\s*(.*?))(?=\s*(?:\(?\s*(?:Arm|Cohort)\s+[\dA-Za-z]+\s*\)?|$))',

    schema_heading = re.compile(r"^\s*(\d+(\.\d+)*)?\s*Schema\s*$", re.IGNORECASE)
    doc = Document(file_path)

    
    schema_heading_found = False
    section_text = []
    


    for para in doc.paragraphs:
        text = para.text.strip()

        if schema_heading.match(text):
            print(f"Found Schema heading: {text}")
            schema_heading_found = True
            continue

        if schema_heading_found:
            if para.style.name.startswith("Heading"):
                print(f"New heading: {text}")
                break
            section_text.append(text)

    return "\n".join(section_text)


def imageTextExtractor(file_path):
    doc = Document(file_path)
    protocol_name = os.path.splitext(os.path.basename(file_path))[0]
    image_folder = f"{protocol_name}-schema"
    os.makedirs(image_folder, exist_ok=True)

    figure_pattern = re.compile(r'(Figure \d+[^\n]*)')
    image_count = 0
    figure_labels = []

    schema_heading = re.compile(r"^\s*(\d+(\.\d+)*)?\s*Schema\s*$", re.IGNORECASE)

    
    schema_heading_found = False
    section_texts = []
    

    print(f"created folder: {image_folder}")

    for para in doc.paragraphs:
        text = para.text.strip()

        if schema_heading.match(text):
            print(f"Found Schema heading: {text}")
            schema_heading_found = True
            continue

        if schema_heading_found:
            if para.style.name.startswith("Heading"):
                print(f"New heading: {text}")
                break

            match = figure_pattern.search(text)
            if match:
                label = match.group().replace(" ", "_").replace("/", "-") + ".png"
                figure_labels.append(label)
                print(f"found image lable: {label}")
    
    image_index = 0

    for rel in doc.part.rels:
            if "image" in doc.part.rels[rel].target_ref:
                if image_index < len(figure_labels):
                    image = doc.part.rels[rel].target_part.blob
                    img = Image.open(BytesIO(image))
                    image_path = os.path.join(image_folder, figure_labels[image_index])
                    img.save(image_path)
                    print(f"saved image: {image_path}")

                    section_text = pytesseract.image_to_string(img)
                    section_texts.append(f"{figure_labels[image_index]}:\n{section_text}\n")
                    print(f"Extracted text from: {figure_labels[image_index]}")
                    image_index += 1

    debug_text_path = os.path.join(image_folder, "section_texts.txt")
    with open(debug_text_path, "w", encoding="utf-8") as f:
        f.writelines(section_texts)
    print(f"Saved extracted text: {debug_text_path}")

    return image_folder


def drugNameExtractor(text):
    print("Extacting drug name...")
    nlp = spacy.load("en_core_web_sm")
    doc = nlp(text)

    for ent in doc.ents:
        print(f"Text: '{ent.text}' | Label: {ent.label_}")

    drug_names = set()

    for ent in doc.ents:
        if ent.label_ == "PRODUCT":
            drug_names.add(ent.text)

    print(f"Extracted drug names: {drug_names}")

    return drug_names


def studyTextExtractor(file_path):
    print("Extracting Study Design section...")
    study_design_heading = re.compile(r"^\s*(\d+(\.\d+)*)?\s*Overall Design\s*$")
    doc = Document(file_path)
    study_design_found = False
    study_text = []

    for para in doc.paragraphs:
        text = para.text.strip()

        if study_design_heading.match(text):
            print(f"Study design -> overall study found: {text}")
            study_design_found = True
            continue

        if study_design_found:
            if para.style.name.startswith("Heading"):
                print(f"Next heading encounter {text}")
                break

            study_text.append(text)
    
    return "\n".join(study_text)


def armCohortInfoExtractore(text):
    print("Extracting Arm and Cohort Info")
    arm_cohort_pattern = re.compile(r'(Arm|Cohort)\s*(\d+)[:,]?\s*(.*)')
    arm_cohort_data = {}

    for match in arm_cohort_pattern.finditer(text):
        label = match.group(1) + " " + match.group(2)
        value = match.group(3)
        arm_cohort_data[label] = value
        print(f"found: {label} -> {value}")

    return arm_cohort_data

def validateDrugExtractor(text_drugs, image_text_drugs):
    print("Validating extracted drug names")
    common_drug = text_drugs.intersection(image_text_drugs)
    missing_in_text = image_text_drugs - text_drugs
    missing_in_image = text_drugs - image_text_drugs

    print(f"drugs found in both: {common_drug}")
    print(f"Drug missing in text: {missing_in_text}")
    print(f"Drug missing in image: {missing_in_image}")
    
    return common_drug, missing_in_text, missing_in_image


def Process_document_drug_info(file_path):
    image_folder = imageTextExtractor(file_path)
    schema_text = schemaExtarctor(file_path)
    study_text = studyTextExtractor(file_path)
    text_drugs = drugNameExtractor(study_text)
    arm_cohort_info = armCohortInfoExtractore(study_text)
    image_text_drugs = drugNameExtractor("\n".join(open(os.path.join(image_folder, "section_texts.txt"), encoding = "utf-8").readlines()))

    validateDrugExtractor(text_drugs, image_text_drugs)

    return{
        "text_drugs": text_drugs,
        "image_text_drug": image_text_drugs,
        "arm_cohort_info": arm_cohort_info
    }

