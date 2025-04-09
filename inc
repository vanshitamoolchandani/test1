import re
from docx import Document

def process_docx(file_path):
    # Initialize results dictionary as per expected structure.
    results = {
        'Inclusion_Criteria': {'total': 0, 'subpoints': {}, 'points': []},
        'Exclusion_Criteria': {'total': 0, 'subpoints': {}, 'points': []}
    }
    
    current_section = None
    current_parent = None

    try:
        doc = Document(file_path)
    except Exception as e:
        print(f"Error opening DOCX file: {e}")
        return None

    print("Starting .docx file processing...\n")
    
    for para in doc.paragraphs:
        line = para.text.strip()
        style = para.style.name if para.style else ""
        print(f"Processing paragraph: '{line}' with style: '{style}'")
        
        # Skip empty paragraphs.
        if not line:
            continue
        
        # Only treat the paragraph as a heading if its style starts with "Heading"
        if style.startswith("Heading"):
            # Check for stopping condition.
            if "Lifestyle considerations" in line:
                print("Encountered 'Lifestyle considerations' heading. Stopping parsing.\n")
                break
            
            # Switch section if the heading text is matched.
            if "Inclusion criteria" in line:
                current_section = 'Inclusion_Criteria'
                print("Switched to Inclusion_Criteria section.\n")
                continue
            elif "Exclusion Criteria" in line or "exclusion Criteria" in line:
                current_section = 'Exclusion_Criteria'
                print("Switched to Exclusion_Criteria section.\n")
                continue
            else:
                # If the heading is not one of our target headings,
                # it is not considered for processing, so reset section.
                current_section = None
                print("Heading not relevant to target sections. Skipping...\n")
                continue

        # Process non-heading paragraphs: look for bullet points.
        bullet_match = re.match(r'^(\d+(?:\.\d+)*)([.)])\s+(.*)', line)
        if bullet_match:
            bullet_num = bullet_match.group(1)  # E.g., "1" or "1.1"
            separator = bullet_match.group(2)     # E.g., "." or ")"
            bullet_text = bullet_match.group(3)   # The text after the bullet
            
            print(f"Detected bullet: {bullet_num}{separator} {bullet_text}")
            
            # If it's a main bullet (no dot in bullet number)
            if '.' not in bullet_num:
                current_parent = bullet_num
                if current_section:
                    results[current_section]['total'] += 1
                    results[current_section]['points'].append(f"{bullet_num}{separator} {bullet_text}")
                    # Initialize subpoints count for this main bullet.
                    results[current_section]['subpoints'][current_parent] = 0
                    print(f"Added main bullet '{bullet_num}' to {current_section}")
                else:
                    print("Main bullet found but no section is currently set. Skipping.")
            else:
                # It's a sub bullet point. Determine its parent by splitting at first dot.
                parent = bullet_num.split('.')[0]
                if current_section and parent in results[current_section]['subpoints']:
                    results[current_section]['subpoints'][parent] += 1
                    print(f"Added sub bullet '{bullet_num}' under parent '{parent}' in {current_section}")
                else:
                    print(f"Sub bullet found but parent '{parent}' does not exist in current section.")
        else:
            print("No bullet pattern matched.\n")
    
    print("\nFinal Results:")
    print(results)
    return results

# Example usage:
# results = process_docx("path/to/your/file.docx")
# print(results)
