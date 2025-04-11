import re
import json

def extract_criteria(file_path):
    # Open the file and read its contents
    with open(file_path, 'r') as file:
        text = file.read()
    print("File loaded. Starting processing.")

    lines = text.splitlines()
    criteria = {}
    current_section = None
    collecting = False

    for i, line in enumerate(lines):
        print(f"Processing line {i}: {line}")
        # Detecting Inclusion or Exclusion Criteria section with optional trailing dot
        section_header = re.match(r'^\s*(\d+)\.(\d+)\.?\s+(Inclusion|Exclusion)\s+Criteria', line, re.IGNORECASE)
        if section_header:
            section_number = f"{section_header.group(1)}.{section_header.group(2)}"
            criteria_type = section_header.group(3).capitalize()
            current_section = criteria_type
            criteria[current_section] = {"count": 0, "subpoints": {}}
            collecting = True
            print(f"Found section: {criteria_type} Criteria with section number {section_number}")
            continue

        if collecting and current_section:
            if "Lifestyle considerations" in line:
                print("Encountered 'Lifestyle considerations'; stopping collection.")
                break

            bullet_match = re.match(r'^\s*(\d+)\.\s+(.*)', line)
            sub_bullet_match = re.match(r'^\s*(\d+)\.(\d+)\s+(.*)', line)

            if bullet_match and not sub_bullet_match:
                main_point = bullet_match.group(1)
                criteria[current_section]["count"] += 1
                if main_point not in criteria[current_section]["subpoints"]:
                    criteria[current_section]["subpoints"][main_point] = 0
                print(f"Found main bullet: {main_point}")

            elif sub_bullet_match:
                main_point = sub_bullet_match.group(1)
                if main_point in criteria[current_section]["subpoints"]:
                    criteria[current_section]["subpoints"][main_point] += 1
                print(f"Found sub bullet under main point {main_point}")

    print("Processing completed.")
    return json.dumps(criteria, indent=4)
