from docx import Document
from docx.document import Document as _Document
from docx.oxml.table import CT_Tbl
from docx.oxml.text.paragraph import CT_P
from docx.table import Table as _Table
from docx.text.paragraph import Paragraph as _Paragraph
import re
from collections import OrderedDict

def countExtractor(file_path):
    """Extract inclusion criteria points count and structure"""
    def iter_block_items(parent):
        if isinstance(parent, _Document):
            parent_elm = parent.element.body
        else:
            raise ValueError("Unsupported parent type")
            
        for child in parent_elm.iterchildren():
            if isinstance(child, CT_P):
                yield _Paragraph(child, parent)
            elif isinstance(child, CT_Tbl):
                yield _Table(child, parent)

    def extract_inclusion_section():
        """Extract text from Inclusion Criteria section"""
        doc = Document(file_path)
        in_section = False
        inclusion_text = []
        
        for block in iter_block_items(doc):
            if isinstance(block, _Paragraph):
                text = block.text.strip()
                style = block.style.name if block.style else None
                
                if not in_section and "inclusion criteria" in text.lower():
                    in_section = True
                    continue
                
                if in_section and style and style.startswith('Heading'):
                    break
                
                if in_section:
                    inclusion_text.append(text)
                    
            elif isinstance(block, _Table) and in_section:
                for row in block.rows:
                    for cell in row.cells:
                        inclusion_text.append(cell.text.strip())
        
        return '\n'.join(inclusion_text)

    def count_points(text):
        """Count main points and subpoints"""
        main_point_regex = re.compile(r'^\s*(\d+)\.', re.MULTILINE)
        sub_point_regex = re.compile(
            r'^\s*([a-z]\.|\(?[a-z]\)|\d+\.\d+\.?|\d+\)|•|\-)\s*',
            re.IGNORECASE | re.MULTILINE
        )
        
        lines = [line.strip() for line in text.split('\n') if line.strip()]
        
        current_main = None
        sub_points_count = 0
        points = OrderedDict()
        
        for line in lines:
            main_match = main_point_regex.match(line)
            if main_match:
                if current_main is not None:
                    points[current_main] = sub_points_count
                current_main = main_match.group(1)
                sub_points_count = 0
            else:
                if current_main is not None and sub_point_regex.match(line):
                    sub_points_count += 1
                    
        if current_main is not None:
            points[current_main] = sub_points_count
            
        return {
            "total": len(points),
            "points": dict(points)
        }

    try:
        section_text = extract_inclusion_section()
        return count_points(section_text)
    except Exception as e:
        print(f"Error in criteria counting: {str(e)}")
        return {"total": 0, "points": {}}
