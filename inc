import re
from collections import OrderedDict
from docx import Document
from docx.document import Document as _Document
from docx.oxml.table import CT_Tbl
from docx.oxml.text.paragraph import CT_P
from docx.table import Table as _Table
from docx.text.paragraph import Paragraph as _Paragraph

def iter_block_items(parent):
    """Generator to iterate through all blocks (paragraphs and tables) in order."""
    if isinstance(parent, _Document):
        parent_elm = parent.element.body
    else:
        raise ValueError("Unsupported parent type")
        
    for child in parent_elm.iterchildren():
        if isinstance(child, CT_P):
            yield _Paragraph(child, parent)
        elif isinstance(child, CT_Tbl):
            yield _Table(child, parent)

def extract_inclusion_criteria_text(file_path):
    """Extract text from Inclusion Criteria section including tables, with debug prints."""
    doc = Document(file_path)
    in_section = False
    found = False
    inclusion_text = []
    
    print("\n=== Starting document processing ===")
    
    for block in iter_block_items(doc):
        if isinstance(block, _Paragraph):
            text = block.text.strip()
            style = block.style.name if block.style else "No Style"
            
            if not in_section:
                print(f"\nChecking paragraph: '{text}'")
                
            # Detect Inclusion Criteria heading
            if not in_section and "inclusion criteria" in text.lower():
                in_section = True
                found = True
                print("\n=== FOUND INCLUSION CRITERIA HEADING ===")
                print(f"Heading text: '{text}'")
                print("Now collecting content...")
                continue
            
            # Stop at next heading
            if in_section and style.startswith('Heading'):
                print(f"\n=== STOPPING AT NEXT HEADING: '{text}' ===")
                in_section = False
                break
            
            if in_section and text:
                inclusion_text.append(text)
                print(f"Added paragraph: '{text}'")
                
        elif isinstance(block, _Table) and in_section:
            print("\nProcessing table in Inclusion Criteria section:")
            table_text = []
            for row_idx, row in enumerate(block.rows):
                for cell_idx, cell in enumerate(row.cells):
                    text = cell.text.strip()
                    if text:
                        table_text.append(text)
                        print(f"Row {row_idx+1}, Cell {cell_idx+1}: '{text}'")
            inclusion_text.extend(table_text)
    
    if not found:
        print("\n=== WARNING: Inclusion Criteria heading not found ===")
    else:
        print("\n=== FINISHED PROCESSING INCLUSION CRITERIA SECTION ===")
    
    return '\n'.join(inclusion_text)

def count_inclusion_criteria_points(text):
    """Count main points and subpoints with detailed debug output."""
    main_point_regex = re.compile(r'^\s*(\d+)\.', re.MULTILINE)
    sub_point_regex = re.compile(
        r'^\s*([a-z]\.|\(?[a-z]\)|\d+\.\d+\.?|\d+\)|•|\-)\s*',
        re.IGNORECASE | re.MULTILINE
    )
    
    print("\n=== STARTING POINT COUNTING ===")
    print("Raw extracted text:")
    print("-------------------")
    print(text)
    print("-------------------")
    
    lines = [line.strip() for line in text.split('\n') if line.strip()]
    print(f"\nFound {len(lines)} non-empty lines to process:")
    for idx, line in enumerate(lines, 1):
        print(f"{idx:2}. {line}")
    
    current_main = None
    sub_points_count = 0
    points = OrderedDict()
    
    print("\nProcessing lines:")
    for line in lines:
        print(f"\nProcessing line: '{line}'")
        
        # Check for main point
        main_match = main_point_regex.match(line)
        if main_match:
            print(f"  MATCHED MAIN POINT: {main_match.group(1)}")
            if current_main is not None:
                print(f"  CLOSING MAIN POINT {current_main} with {sub_points_count} sub-points")
                points[current_main] = sub_points_count
            current_main = main_match.group(1)
            sub_points_count = 0
            print(f"  NEW MAIN POINT: {current_main}")
            continue
        
        # Check for sub-points under current main
        if current_main is not None:
            sub_match = sub_point_regex.match(line)
            if sub_match:
                print(f"  MATCHED SUB-POINT: {sub_match.group(1)}")
                sub_points_count += 1
                print(f"  SUB-POINT COUNT FOR {current_main}: {sub_points_count}")
            else:
                print("  NO SUB-POINT MATCH")
    
    # Add last main point
    if current_main is not None:
        print(f"\nFinalizing last main point {current_main}")
        points[current_main] = sub_points_count
    
    print("\n=== COUNTING RESULTS ===")
    print(f"Total main points found: {len(points)}")
    for point, count in points.items():
        print(f"Main point {point} has {count} sub-points")
    
    return points

# Example usage
file_path = "your_document.docx"

try:
    print("====== STARTING PROCESS ======")
    
    # Step 1: Extract text from DOCX
    print("\n*** PHASE 1: TEXT EXTRACTION ***")
    inclusion_text = extract_inclusion_criteria_text(file_path)
    
    # Step 2: Analyze points
    print("\n\n*** PHASE 2: POINT COUNTING ***")
    points = count_inclusion_criteria_points(inclusion_text)
    
    # Final output
    print("\n\n=== FINAL RESULTS ===")
    print(f"Inclusion Criteria: count: {len(points)}")
    for point, sub_count in points.items():
        print(f"        {point}: {sub_count}")
        
except Exception as e:
    print(f"\n!!! ERROR: {str(e)}")
