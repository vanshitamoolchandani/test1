Clinical Trial Protocol Extraction Tool Documentation
This Python script extracts key information from clinical trial protocol documents (.docx format) using natural language processing, OCR, and structured document analysis.

Core Functions:
read_word_file(file_path)
Reads a Word document and returns non-empty paragraphs.

Parameters: file_path (str) - Path to .docx file

Returns: List of paragraph texts

title_extractor(file_path)
Extracts study title using keyword matching and section analysis.

Looks for keywords: "Title:", "Study Title:", "Open-label", "Randomized", etc.

Returns: Extracted title string

amendment_date_extractor(file_path)
Finds latest amendment number and date from document tables.

Returns: Dict with amendment (int) and date (str in "dd Month YYYY" format)

age_extractor(file_path)
Extracts age range from inclusion criteria section.

Recognizes patterns: "18-65 years", "≥18 years", etc.

Returns: Dict with min and max ages

weight_extractor(file_path)
Extracts weight/BMI criteria from inclusion/exclusion sections.

Returns: List of weight/BMI requirement strings

participant_count_extractor(file_path)
Extracts participant counts from "NUMBER OF PARTICIPANTS" section.

Uses spaCy for cardinal number detection

Returns: List of dicts with text and value

ratio_extractor(file_path)
Extracts treatment ratios (e.g., "1:1:1")

Returns: List of ratio-containing sentences

therapeutic_area_extractor(title)
Identifies medications/diseases from study title using NER models.

Uses Hugging Face models: d4data/biomedical-ner-all and raynardj/ner-disease...

Returns: Dict with medications and diseases lists

image_text_extractor(file_path)
Extracts text from "Schema" section images using OCR (Tesseract).

Saves images to schema/{protocol}-schema/ folder

Returns: Path to extracted images/text

arm_cohort_table_extractor(file_path)
Extracts treatment arm tables using heading pattern matching.

Recognizes headings like "Description of Study Arms"

Returns: Dict of tables with extracted data

inclusion_count_extractor(file_path)
Counts inclusion criteria items using document structure analysis.

Identifies list hierarchies (main points vs subpoints)

Returns: Structured count dictionary

exclusion_count_extractor(file_path)
Counts exclusion criteria items (similar to inclusion counter)

Returns: Structured count dictionary

process_protocol(file_path)
Main pipeline that extracts medications/diseases while excluding those mentioned in exclusion criteria.

Returns: Dict with filtered medications and diseases

Key Dependencies:
Document Processing: python-docx, docx

NLP/ML: spacy, transformers, pytesseract

Utilities: pandas, dateutil, re, PIL, cv2

