import re
import json
from docx import Document

def process_docx(file_path):
    results = {
        'Inclusion_Criteria': {'total': 0, 'subpoints': {}, 'points': []},
        'Exclusion_Criteria': {'total': 0, 'subpoints': {}, 'points': []}
    }

    headings_order = []
    current_section = None
    current_parent = None

    try:
        doc = Document(file_path)
        paragraphs = doc.paragraphs
    except Exception as e:
        print(f"Error opening DOCX file: {e}")
        return json.dumps({"error": str(e)})

    print("Starting .docx file processing...")

    for para in paragraphs:
        line = para.text.strip()
        print(f"Processing line: {line}")

        if not line:
            continue

        # Stop when Lifestyle considerations is encountered
        if "Lifestyle considerations" in line:
            print("Encountered 'Lifestyle considerations' heading. Stopping parsing.")
            break

        # Detect section headings
        if "Inclusion criteria" in line:
            current_section = "Inclusion_Criteria"
            headings_order.append("Inclusion criteria")
            print("Switched to Inclusion Criteria section.")
            continue
        elif "Exclusion Criteria" in line or "exclusion Criteria" in line:
            current_section = "Exclusion_Criteria"
            headings_order.append("Exclusion Criteria")
            print("Switched to Exclusion Criteria section.")
            continue

        # Match bullets (numbered or sub-numbered)
        bullet_match = re.match(r'^(\d+(?:\.\d+)*)([.)])\s+(.*)', line)
        if bullet_match:
            bullet_num = bullet_match.group(1)
            separator = bullet_match.group(2)
            bullet_text = bullet_match.group(3)

            print(f"Detected bullet: {bullet_num}{separator} {bullet_text}")

            # Main bullet
            if '.' not in bullet_num:
                current_parent = bullet_num
                if current_section:
                    results[current_section]['total'] += 1
                    results[current_section]['points'].append(f"{bullet_num}{separator} {bullet_text}")
                    results[current_section]['subpoints'][current_parent] = 0
                    print(f"Main bullet added under {current_section}: {bullet_num}")
            else:
                # Sub bullet
                parent = bullet_num.split('.')[0]
                if current_section and parent in results[current_section]['subpoints']:
                    results[current_section]['subpoints'][parent] += 1
                    print(f"Sub bullet added under {current_section}, parent {parent}")
        else:
            print("No bullet point matched.")

    # Final debug output
    print("\nFinal Result:")
    print(json.dumps(results, indent=4))

    return json.dumps(results, indent=4)

# Example usage:
# output = process_docx("path/to/your/file.docx")
# print(output)
