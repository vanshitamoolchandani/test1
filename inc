from docx import Document
import re

def extract_criteria_points(file_path):
    """
    Final corrected version with proper syntax and enhanced logging
    """
    doc = Document(file_path)
    
    # Section heading patterns
    inclusion_pattern = re.compile(r'inclusion\s+criteria', re.IGNORECASE)
    exclusion_pattern = re.compile(r'exclusion\s+criteria', re.IGNORECASE)
    lifestyle_pattern = re.compile(r'lifestyle\s+considerations', re.IGNORECASE)
    
    # State tracking
    current_section = None
    results = {
        'Inclusion_Criteria': {
            'total_count': 0,
            'subpoint_count': {},
            'points': []
        },
        'Exclusion_Criteria': {
            'total_count': 0,
            'subpoint_count': {},
            'points': []
        }
    }
    
    print(f"\n{'='*40}\nProcessing document: {file_path}\n{'='*40}")

    for para_idx, para in enumerate(doc.paragraphs):
        text = para.text.strip()
        original_text = para.text
        
        # Section detection
        if inclusion_pattern.search(text):
            current_section = 'Inclusion_Criteria'
            print(f"\n{'='*20} FOUND INCLUSION HEADER {'='*20}")
            continue
        elif exclusion_pattern.search(text):
            current_section = 'Exclusion_Criteria'
            print(f"\n{'='*20} FOUND EXCLUSION HEADER {'='*20}")
            continue
        elif lifestyle_pattern.search(text):
            print(f"\n{'='*20} FOUND LIFESTYLE HEADER (STOPPING) {'='*20}")
            break

        # Process points in current section
        if current_section and current_section in results:
            # Improved point pattern matching
            point_match = re.match(
                r'^(\s*[\(\[\{]*\s*(\d+[a-z]*|[a-zA-Z])(\.\d+[a-z]*)*[\)\]\}\.\-\s]*(.*)', 
                text
            )
            
            if point_match and point_match.group(4).strip():
                main_point = point_match.group(2)
                sub_points = point_match.group(3).split('.')[1:] if point_match.group(3) else []
                point_text = point_match.group(4).strip()

                # Update counts
                results[current_section]['total_count'] += 1
                if main_point not in results[current_section]['subpoint_count']:
                    results[current_section]['subpoint_count'][main_point] = 0
                results[current_section]['subpoint_count'][main_point] += len(sub_points)
                
                # Store point details
                results[current_section]['points'].append({
                    'main_point': main_point,
                    'sub_points': sub_points,
                    'text': point_text
                })
                
                print(f"✅ Added point to {current_section}:")
                print(f"   Main: {main_point}, Subs: {sub_points}")
                print(f"   Text: {point_text[:60]}...")

    # Print final results
    print("\n" + "="*40 + "\nFinal Results:\n" + "="*40)
    for section in results:
        print(f"\n{section}:")
        print(f"Total points: {results[section]['total_count']}")
        print(f"Subpoint distribution: {results[section]['subpoint_count']}")
        print(f"First 3 points: {[p['text'][:30]+'...' for p in results[section]['points'][:3]]}")
    
    return results
