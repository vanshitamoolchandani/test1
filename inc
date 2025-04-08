from docx import Document
import re

def extract_criteria_points(file_path):
    """
    Enhanced version that:
    - Counts both main points and subpoints
    - Provides detailed output structure
    - Better handles numbered points
    """
    doc = Document(file_path)
    
    # Patterns to identify section headings
    inclusion_pattern = re.compile(r'^\s*\d*\.?\s*inclusion\s+criteria\s*$', re.IGNORECASE)
    exclusion_pattern = re.compile(r'^\s*\d*\.?\s*exclusion\s+criteria\s*$', re.IGNORECASE)
    lifestyle_pattern = re.compile(r'^\s*\d*\.?\s*lifestyle\s+considerations\s*$', re.IGNORECASE)
    
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
    
    print(f"\nProcessing document: {file_path}")
    
    for para in doc.paragraphs:
        text = para.text.strip()
        
        # Check for section headers (only at paragraph start)
        if inclusion_pattern.match(text):
            current_section = 'Inclusion_Criteria'
            print(f"\nFound Inclusion Criteria heading: {text}")
            continue
            
        elif exclusion_pattern.match(text):
            current_section = 'Exclusion_Criteria'
            print(f"\nFound Exclusion Criteria heading: {text}")
            continue
            
        elif lifestyle_pattern.match(text):
            print(f"\nFound Lifestyle Considerations heading: {text}")
            break  # Stop processing after this
            
        # Process points if in a section
        if current_section and text:
            # Match numbered points (both main and subpoints)
            point_match = re.match(r'^\s*(\d+)(?:\.(\d+))?[\.\)\s\-]*(.*)', text)
            
            if point_match:
                main_num = point_match.group(1)
                sub_num = point_match.group(2)
                point_text = point_match.group(3).strip()
                
                if point_text:  # Only count if there's actual text
                    results[current_section]['points'].append(text)
                    results[current_section]['total_count'] += 1
                    
                    # Track subpoints
                    if sub_num:
                        if main_num not in results[current_section]['subpoint_count']:
                            results[current_section]['subpoint_count'][main_num] = 0
                        results[current_section]['subpoint_count'][main_num] += 1
                    
                    print(f"Added to {current_section}: {text[:60]}...")
    
    print("\n=== Extraction Results ===")
    for section, data in results.items():
        print(f"\n{section}:")
        print(f"Total points: {data['total_count']}")
        print(f"Subpoint distribution: {data['subpoint_count']}")
        print(f"First 3 points: {data['points'][:3]}...")
    
    return results
