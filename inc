from docx import Document
import re

def extract_criteria_points(file_path):
    """
    Final version with improved point detection and debugging
    """
    doc = Document(file_path)
    
    # Section heading patterns (more flexible matching)
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
        
        # Debug: Show raw paragraph text
        print(f"\nParagraph {para_idx+1} (SECTION: {current_section}):")
        print(f"Raw text: {original_text!r}")
        
        # Detect section headers
        if not current_section:
            if inclusion_pattern.search(text):
                current_section = 'Inclusion_Criteria'
                print(f"\n{'='*20} FOUND INCLUSION HEADER {'='*20}")
                continue
            elif exclusion_pattern.search(text):
                current_section = 'Exclusion_Criteria'
                print(f"\n{'='*20} FOUND EXCLUSION HEADER {'='*20}")
                continue
        else:
            if lifestyle_pattern.search(text):
                print(f"\n{'='*20} FOUND LIFESTYLE HEADER (STOPPING) {'='*20}")
                break

        # Process points in current section
        if current_section and current_section in results:
            # Enhanced point pattern matching
            point_match = re.match(r'^(\s*[\(\[\{]*\s*(\d+[a-z]*|[a-zA-Z])(\.\d+[a-z]*)*[\)\]\}\.\-\s]*)(.*)', text)
            
            if point_match:
                full_number = point_match.group(2) + (point_match.group(3) if point_match.group(3) else ''
                point_text = point_match.group(4).strip()
                numbering = point_match.group(1).strip()
                
                if point_text:
                    # Count main points and subpoints
                    main_point = point_match.group(2)
                    sub_points = point_match.group(3).split('.')[1:] if point_match.group(3) else []
                    
                    if not sub_points:  # Main point
                        results[current_section]['total_count'] += 1
                        results[current_section]['subpoint_count'][main_point] = 0
                    else:  # Subpoint
                        results[current_section]['subpoint_count'][main_point] = len(sub_points)
                    
                    results[current_section]['points'].append({
                        'numbering': numbering,
                        'text': point_text,
                        'main_point': main_point,
                        'sub_points': sub_points
                    })
                    
                    print(f"✅ Added point to {current_section}:")
                    print(f"   Numbering: {numbering}")
                    print(f"   Text: {point_text[:60]}...")
                    continue
                else:
                    print(f"🚨 Empty point detected: {text}")
            elif text:  # Non-empty text without numbering
                print(f"⚠️  Unstructured text in {current_section}: {text[:60]}...")

    print("\n" + "="*40 + "\nFinal Results:\n" + "="*40)
    for section in results:
        print(f"\n{section}:")
        print(f"Total points: {results[section]['total_count']}")
        print(f"Subpoint distribution: {dict(results[section]['subpoint_count'])}")
        print(f"Sample points: {[p['text'][:30]+'...' for p in results[section]['points'][:3]]}")
    
    return results
