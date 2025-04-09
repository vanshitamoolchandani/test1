from docx import Document
import re

def extract_criteria_points(file_path):
    """
    Updated version that:
     - Checks for header patterns and immediately processes numbered bullets.
     - Counts numbered bullets right after the header is encountered.
     - Prints detailed debug logs to follow the flow.
    """
    doc = Document(file_path)
    
    header_patterns = {
        'inclusion': re.compile(r'^(\d+[\.\d]*\s*)?inclusion\s+criteria\b.*', re.IGNORECASE),
        'exclusion': re.compile(r'^(\d+[\.\d]*\s*)?exclusion\s+criteria\b.*', re.IGNORECASE),
        'lifestyle': re.compile(r'^(\d+[\.\d]*\s*)?lifestyle\s+considerations\b.*', re.IGNORECASE)
    }
    
    results = {
        'Inclusion_Criteria': {'total': 0, 'subpoints': {}, 'points': []},
        'Exclusion_Criteria': {'total': 0, 'subpoints': {}, 'points': []}
    }
    
    current_section = None
    in_lifestyle = False
    
    print(f"\n{'='*40}\nProcessing document: {file_path}\n{'='*40}")
    
    for index, para in enumerate(doc.paragraphs):
        text = para.text.strip()
        print(f"\nParagraph {index+1}: '{text}'")
        
        # Check header patterns: if found, update current_section immediately.
        # (Note: We still want to count numbered bullets in this paragraph.)
        header_detected = False
        for key, pattern in header_patterns.items():
            if pattern.match(text):
                if key == 'lifestyle':
                    print(f"\n🚩 Lifestyle Considerations found: {text}")
                    in_lifestyle = True
                    header_detected = True
                    break
                elif key == 'inclusion':
                    current_section = 'Inclusion_Criteria'
                    print(f"\n🔍 Found Inclusion Criteria header: {text}")
                    header_detected = True
                    break
                elif key == 'exclusion':
                    current_section = 'Exclusion_Criteria'
                    print(f"\n🔍 Found Exclusion Criteria header: {text}")
                    header_detected = True
                    break
        
        if in_lifestyle:
            break

        # Now check for numbered bullet points in this paragraph,
        # regardless of whether this paragraph was a header.
        if current_section and text:
            # This regex finds a leading number, optionally followed by one or more subpoint numbers.
            main_match = re.match(r'^(\d+).*?\s*(.*)', text)
            if main_match:
                main_num = main_match.group(1)
                point_text = main_match.group(2).strip()
                print(f"➡️ Main number: {main_num}, Point text: '{point_text}'")
                
                # Find all subpoints using findall; this pattern captures numbers after a dot.
                sub_nums = re.findall(r'\.(\d+)', text)
                print(f"🔍 Found subpoints for paragraph {index+1}: {sub_nums}")
                
                if point_text:
                    results[current_section]['total'] += 1
                    results[current_section]['points'].append(point_text)
                    
                    if sub_nums:
                        if main_num in results[current_section]['subpoints']:
                            results[current_section]['subpoints'][main_num] += len(sub_nums)
                        else:
                            results[current_section]['subpoints'][main_num] = len(sub_nums)
                        print(f"✅ Updated subpoints for main number {main_num}: {results[current_section]['subpoints'][main_num]}")
                    else:
                        print("ℹ️ No subpoints found for this paragraph.")
                else:
                    print("⚠️ Skipping paragraph due to empty point text.")
            else:
                # If header was detected but no bullet match, log it.
                if header_detected:
                    print("ℹ️ Header detected but no bullet point found in this paragraph.")
                else:
                    print("⚠️ No numbered bullet found in this paragraph.")
    
    print("\n" + "="*40 + "\nFinal Results:" + "="*40)
    for section, data in results.items():
        print(f"\n{section}:")
        print(f"Total points: {data['total']}")
        print(f"Subpoints: {data['subpoints']}")
        print(f"First 3 points: {data['points'][:3]}")
    
    return results
