from docx import Document
import re

def extract_criteria_points(file_path):
    """
    Final working version with:
    - Proper header detection
    - Accurate point counting
    - Subpoint tracking
    """
    doc = Document(file_path)
    
    # Enhanced header patterns with numbering support
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

    for para in doc.paragraphs:
        text = para.text.strip()
        
        # Detect section headers
        if header_patterns['lifestyle'].match(text):
            print(f"\n🚩 Lifestyle Considerations found: {text}")
            in_lifestyle = True
            break
            
        if not in_lifestyle:
            if header_patterns['inclusion'].match(text):
                current_section = 'Inclusion_Criteria'
                print(f"\n🔍 Found Inclusion Criteria: {text}")
                continue
                
            if header_patterns['exclusion'].match(text):
                current_section = 'Exclusion_Criteria'
                print(f"\n🔍 Found Exclusion Criteria: {text}")
                continue

        # Process points in current section
        if current_section and not in_lifestyle and text:
            # Match numbered points (main and subpoints)
            match = re.match(r'^(\d+)(?:\.(\d+))*(?:\.|\s|\)|-|\)|:)*\s*(.*)', text)
            
            if match and match.group(3).strip():
                main_num = match.group(1)
                sub_nums = match.groups()[1:-1]  # Get all subpoint numbers
                point_text = match.group(3).strip()
                
                # Update counts
                results[current_section]['total'] += 1
                results[current_section]['points'].append(point_text)
                
                # Track subpoints
                if sub_nums:
                    if main_num not in results[current_section]['subpoints']:
                        results[current_section]['subpoints'][main_num] = 0
                    results[current_section]['subpoints'][main_num] += len(sub_nums)
                
                print(f"📝 Added to {current_section}: {point_text[:60]}...")

    print("\n" + "="*40 + "\nResults:" + "="*40)
    for section, data in results.items():
        print(f"\n{section}:")
        print(f"Total points: {data['total']}")
        print(f"Subpoints: {data['subpoints']}")
        print(f"First 3 points: {data['points'][:3]}")
    
    return results
