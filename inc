from docx import Document
from docx.document import Document as _Document
from docx.oxml.table import CT_Tbl
from docx.oxml.text.paragraph import CT_P
from docx.table import Table as _Table
from docx.text.paragraph import Paragraph as _Paragraph
import re

def iter_block_items(parent):
    if isinstance(parent, _Document):
        parent_elm = parent.element.body
    for child in parent_elm.iterchildren():
        if isinstance(child, CT_P):
            yield _Paragraph(child, parent)
        elif isinstance(child, CT_Tbl):
            yield _Table(child, parent)

def get_style_details(paragraph):
    style_info = {
        'text': paragraph.text.strip(),
        'style_name': paragraph.style.name if paragraph.style else None,
        'is_list': False,
        'list_level': None,
        'numbering_format': None,
        'is_table': False
    }

    if paragraph._p.pPr is not None and paragraph._p.pPr.numPr is not None:
        num_pr = paragraph._p.pPr.numPr
        style_info['is_list'] = True
        ilvl = num_pr.ilvl
        style_info['list_level'] = ilvl.val if ilvl is not None else 0

    return style_info

def extract_inclusion_criteria(file_path):
    doc = Document(file_path)
    inclusion_data = {
        "count": 0,
        "details": {},
        "table_found": False
    }
    in_inclusion = False
    main_points = []
    current_main_index = -1
    numbering_pattern = re.compile(r'^\s*(\d+[.)]|[a-z]\)|[A-Z]\.)', re.IGNORECASE)
    
    for block in iter_block_items(doc):
        if isinstance(block, _Paragraph):
            text = block.text.strip()
            style_name = block.style.name if block.style else None

            if style_name == 'Heading 1' and 'inclusion criteria' in text.lower():
                in_inclusion = True
                continue
            elif in_inclusion and style_name == 'Heading 1':
                break

            if not in_inclusion:
                continue

            details = get_style_details(block)
            if details['is_list'] and details['style_name'] == 'List Paragraph':
                level = details['list_level']
                if level == 0:
                    main_points.append({"text": details['text'], "sub_points": 0})
                    current_main_index = len(main_points) - 1
                elif level == 1 and current_main_index != -1:
                    if numbering_pattern.match(details['text']):
                        main_points[current_main_index]["sub_points"] += 1
        elif isinstance(block, _Table) and in_inclusion:
            inclusion_data['table_found'] = True

    inclusion_data['count'] = len(main_points)
    inclusion_data['details'] = {i+1: mp['sub_points'] for i, mp in enumerate(main_points)}
    
    return inclusion_data
