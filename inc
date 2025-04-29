import json
import re
from docx import Document
from docx.document import Document as _Document
from docx.oxml.table import CT_Tbl
from docx.oxml.text.paragraph import CT_P
from docx.table import Table as _Table
from docx.text.paragraph import Paragraph as _Paragraph

def countExtractor(file_path):
    """Extract style information for inclusion criteria section"""
    def iter_block_items(parent):
        if isinstance(parent, _Document):
            parent_elm = parent.element.body
        for child in parent_elm.iterchildren():
            if isinstance(child, CT_P):
                yield _Paragraph(child, parent)
            elif isinstance(child, CT_Tbl):
                yield _Table(child, parent)


def get_style_details(paragraph, doc):
    style_info = {
        'text': paragraph.text.strip(),
        'style_name': paragraph.style.name if paragraph.style else None,
        'is_list': False,
        'list_level': None,
        'numbering_format': None,
        'is_table': False
    }

    # Check if paragraph is part of a list
    if paragraph._p.pPr is not None and paragraph._p.pPr.numPr is not None:
        num_pr = paragraph._p.pPr.numPr
        style_info['is_list'] = True

        # List level (ilvl)
        ilvl = num_pr.ilvl
        style_info['list_level'] = int(ilvl.val) if ilvl is not None else 0

        try:
            num_id = num_pr.numId.val
            numbering = doc.part.numbering_part.numbering_definitions._numbering
            abstract_num_id = None
            for num in numbering.findall('.//w:num', numbering.nsmap):
                if int(num.find('w:numId', numbering.nsmap).get('{http://schemas.openxmlformats.org/wordprocessingml/2006/main}val')) == num_id:
                    abstract_num_id = num.find('w:abstractNumId', numbering.nsmap).get('{http://schemas.openxmlformats.org/wordprocessingml/2006/main}val')
                    break

            if abstract_num_id:
                for abstract in numbering.findall('.//w:abstractNum', numbering.nsmap):
                    if abstract.get('{http://schemas.openxmlformats.org/wordprocessingml/2006/main}abstractNumId') == abstract_num_id:
                        num_fmt = abstract.find('.//w:numFmt', numbering.nsmap)
                        style_info['numbering_format'] = num_fmt.get('{http://schemas.openxmlformats.org/wordprocessingml/2006/main}val') if num_fmt is not None else 'unknown'
                        break
        except Exception as e:
            style_info['numbering_format'] = f'error: {str(e)}'

    return style_info


    try:
        doc = Document(file_path)
        styles_data = []
        in_section = False
        table_found = False
        inclusion_criteria_found = False

        inclusion_data = {}
        main_point_index = 0
        current_main_point = None

        print("Starting processing document...")

        for block in iter_block_items(doc):
            print("Processing a block...")  # Debug: Indicate a block is being processed
            if isinstance(block, _Paragraph):
                text = block.text.strip()
                print(f"Paragraph text: {text}")  # Debug: Print paragraph text

                if not inclusion_criteria_found and "inclusion criteria" in text.lower():
                    inclusion_criteria_found = True
                    in_section = True
                    print("Inclusion criteria section found.")
                    continue

                if in_section:
                    if "excluding criteria" in text.lower() or block.style.name.startswith('Heading'):
                        print("Exclusion criteria or heading encountered.  Breaking section processing.")
                        break

                    style_info = get_style_details(block)
                    styles_data.append(style_info)
                    print(f"Style info: {style_info}") # Debug: show style info

                    if style_info['style_name'] == 'List Paragraph' and style_info['list_level'] == 0:
                        main_point_index += 1
                        inclusion_data[str(main_point_index)] = 0
                        current_main_point = str(main_point_index)
                        print(f"New main point found: {current_main_point}")
                    elif style_info['style_name'] == 'List Paragraph' and style_info['list_level'] == 1:
                        if current_main_point:
                            if re.match(r"^\s*[\dA-Za-z][\.\)]", text):
                                inclusion_data[current_main_point] += 1
                                print(f"Subpoint counted for main point {current_main_point}")
                            else:
                                print(f"Subpoint ignored (no number/letter bullet): {text}")
                    else:
                        print("Paragraph ignored (not a relevant list paragraph)")

            elif isinstance(block, _Table) and in_section:
                table_found = True
                print("Table found in inclusion criteria section.")

        output = {}
        total_main_points = main_point_index
        output["Inclusion Criteria"] = {"count": total_main_points, **inclusion_data}
        if table_found:
            output["Inclusion Criteria"]["table_found"] = True

        print("Finished processing document.")
        return output

    except Exception as e:
        print(f"Error extracting styles: {str(e)}")
        return {}


def main():
    file_path = "/home/vanshita.m297/Desktop/PS updated/Protocol-Scrapper/data/77242113PSO3001---Protocol.docx"  # replace with the actual path
    result = countExtractor(file_path)
    print(json.dumps(result, indent=2))

if __name__ == "__main__":
    main()
