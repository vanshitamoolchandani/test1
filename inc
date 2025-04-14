from docx import Document
from docx.document import Document as _Document
from docx.oxml.table import CT_Tbl
from docx.oxml.text.paragraph import CT_P
from docx.table import Table as _Table
from docx.text.paragraph import Paragraph as _Paragraph

def analyze_inclusion_criteria(doc_path):
    def iter_block_items(parent):
        if isinstance(parent, _Document):
            parent_elm = parent.element.body
        for child in parent_elm.iterchildren():
            if isinstance(child, CT_P):
                yield _Paragraph(child, parent)
            elif isinstance(child, CT_Tbl):
                yield _Table(child, parent)

    doc = Document(doc_path)
    in_inclusion = False
    current_main_point = 0
    results = {
        'total_points': 0,
        'subpoints': {}
    }

    for block in iter_block_items(doc):
        if isinstance(block, _Paragraph):
            text = block.text.strip().lower()
            
            # Start tracking after inclusion criteria header
            if 'inclusion criteria' in text and not in_inclusion:
                in_inclusion = True
                continue
                
            if in_inclusion and 'exclusion criteria' in text:
                break  # Stop at exclusion criteria

            if in_inclusion:
                # Check if it's a list paragraph
                if block.style.name == 'List Paragraph':
                    is_list = True
                    level = 0
                    
                    # Get list level
                    if block._p.pPr and block._p.pPr.numPr:
                        ilvl = block._p.pPr.numPr.ilvl
                        if ilvl is not None:
                            level = ilvl.val
                    
                    # Main point (level 0)
                    if level == 0:
                        current_main_point += 1
                        results['total_points'] += 1
                        results['subpoints'][current_main_point] = 0
                    # Sub-point (level >= 1)
                    elif level >= 1 and current_main_point > 0:
                        results['subpoints'][current_main_point] += 1

    return results

# Usage example
result = analyze_inclusion_criteria('your_document.docx')
print({
    "Inclusion Criteria": {
        "Total Points": result['total_points'],
        "Sub-points Breakdown": result['subpoints']
    }
})
