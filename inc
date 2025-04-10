import re
import json
from docx import Document

def extract_numbered_points(docx_filename, target_headings):
    def is_bullet_point(text):
        return re.match(r"^\d+(\.\d+)*\s", text.strip()) is not None

    def clean_and_process_section(paragraphs):
        section_data = {"point_count": 0, "points": []}
        current_point = None

        for para in paragraphs:
            text = para.text.strip()
            if not is_bullet_point(text):
                continue  # Skip non-bullet lines

            level = text.count('.')  # 1 = main point, 2 = sub-point, etc.

            if level == 1:
                # Save previous point if exists
                if current_point is not None:
                    section_data["points"].append(current_point)
                # Start new point
                current_point = {
                    "point_text": text,
                    "sub_point_count": 0
                }
                section_data["point_count"] += 1

            elif level >= 2 and current_point is not None:
                current_point["sub_point_count"] += 1

        # Save last point
        if current_point is not None:
            section_data["points"].append(current_point)

        return section_data

    doc = Document(docx_filename)
    results = {}

    current_section_heading = None
    current_section_paragraphs = []

    for para in doc.paragraphs:
        text = para.text.strip()
        if para.style.name.startswith("Heading") and text in target_headings:
            if current_section_heading:
                # Process previous section
                results[current_section_heading] = clean_and_process_section(current_section_paragraphs)

            current_section_heading = text
            current_section_paragraphs = []
        elif current_section_heading:
            current_section_paragraphs.append(para)

    if current_section_heading:
        results[current_section_heading] = clean_and_process_section(current_section_paragraphs)

    return results

# Usage
if __name__ == "__main__":
    target_headings = ["Inclusion Criteria", "Exclusion criteria"]
    file_path = "your_file.docx"  # Replace with your actual file path
    extracted_data = extract_numbered_points(file_path, target_headings)
    print(json.dumps(extracted_data, indent=2))
